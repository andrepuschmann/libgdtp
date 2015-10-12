#include "implicitack_scheduler.h"

namespace libgdtp
{

ImplicitAckScheduler::~ImplicitAckScheduler()
{
    LOG_INFO("Combined frames: " << counter_);
}

void ImplicitAckScheduler::add_flow(FlowBase* flow)
{
    boost::lock_guard<boost::mutex> lock(mutex_);
    // only add connections to queue if they are not serviced yet
    if (serviced_flows_.find(flow->get_src_id()) == serviced_flows_.end()) {
        serviced_flows_.insert(flow->get_src_id());
        queue_.push(flow);
    }
    not_empty_cond_.notify_one();
}

FlowBase* ImplicitAckScheduler::get_next_flow()
{
    boost::mutex::scoped_lock lock(mutex_);
    while (queue_.empty()) {
        not_empty_cond_.wait(lock);
    }
    flow_ = queue_.top();
    queue_.pop();
    serviced_flows_.erase(flow_->get_src_id());
    return flow_;
}

bool ImplicitAckScheduler::has_waiting_flow(void)
{
    boost::mutex::scoped_lock lock(mutex_);
    return (not queue_.empty());
}

void ImplicitAckScheduler::get_pdus_for_below(PduVector &pdus)
{
    this->get_next_flow();
    assert(flow_ != NULL);

    Pdu pdu;
    flow_->get_frame_for_below(pdu);
    pdus.push_back(pdu);

    if (pdu.get_type() == ACK) {
        // try to enqueue another PDU if this is an ACK
        //boost::this_thread::sleep(boost::posix_time::microseconds(10));
        // FIXME: check locking here
        //boost::mutex::scoped_lock lock(mutex_);
        if (not queue_.empty()) {
            flow_->frame_transmitted();
            this->get_next_flow();
            flow_->get_frame_for_below(pdu);
            pdus.push_back(pdu);
            counter_++;
        }
    } else {
        //assert(queue_.empty() == true);
    }
}

ASSIGN_LOGPTR(ImplicitAckScheduler::logger_, ImplicitAckScheduler::get_name())

} // namespace libgdtp

