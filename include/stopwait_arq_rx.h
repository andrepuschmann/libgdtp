#ifndef STOPWAIT_ARQ_RX_H
#define STOPWAIT_ARQ_RX_H

#include <mutex>
#include <condition_variable>
#include <atomic>
#include "arq_base.h"
#include "logger.h"

namespace libgdtp {

class FlowBase;

class StopWaitArqRx : public ArqBase
{
public:
    StopWaitArqRx(FlowBase* flow, size_t buffer_size);
    ~StopWaitArqRx(void) {};

    void handle_pdu_from_below(Pdu& pdu);
    Pdu get_ack_for_data_frame(Pdu &pdu, const SeqNo seqno);
    void frame_transmitted() {};

private:
    // member functions defined in arq_base
    void handle_data_pdu(Pdu& pdu);
    static std::string get_name(void) { return "StopWaitArqRx"; }

    // member variables ..
    std::atomic<SeqNo> last_seq_no_; ///< seqno of last PDU
    std::atomic<SeqNo> expected_seq_no_; ///< expected seqno of next PDU

    DECLARE_LOGPTR(logger_)
};

}

#endif // STOPWAIT_ARQ_RX_H
