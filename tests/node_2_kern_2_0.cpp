#include "node_2_kern_2.hpp"

#ifndef __HLS__
#define hc_S0_barrier_increment 7
void S0_barrier_increment(gc_AMToken_t token);

static gasnet_handlerentry_t handlers[] =
{
	{
		hc_S0_barrier_increment,
		(void(*)())S0_barrier_increment
	}
};
#endif

extern "C"{
void kern0(
    short id,
    galapagos::interface <word_t> * in,
    #ifdef __HLS__
    galapagos::interface<word_t> * out,
    int * handler_ctrl,
    #else
    galapagos::interface<word_t> * out
    #endif
){
    #pragma HLS INTERFACE axis port=in
    #pragma HLS INTERFACE axis port=out
    #pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE m_axi port=handler_ctrl depth=32 offset=0

    galapagos::stream_packet <word_t> axis_word;

    SAFE_COUT("Entering kern0\n");

    TIMESTAMP(kern0_0)
    #ifdef __HLS__
    shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out, handler_ctrl);
    #else
    shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out);
    #endif
    TIMESTAMP(kern0_1)

    kernel.init();
    TIMESTAMP(kern0_2)
    #ifndef __HLS__
    ATOMIC_ACTION(kernel.attach(handlers, 1, SEGMENT_SIZE));
    #endif
    TIMESTAMP(kern0_3)

    kernel.barrier_wait();
    TIMESTAMP(kern0_4)

    word_t payload;
    memcpy(&payload, "GAScore", 8);

    TIMESTAMP(kern0_5)
    ATOMIC_ACTION(kernel.sendMediumAM_normal(1, 2, 0, 0, nullptr, 8, (word_t*)(&payload)));
    TIMESTAMP(kern0_6)
    SAFE_COUT(COLOR(Color::FG_RED, dec, "kern0: sending payload\n"));

    kernel.wait_reply(1); // from medium message
    TIMESTAMP(kern0_7)
    SAFE_COUT(COLOR(Color::FG_RED, dec, "kern0: got reply\n"));

    ATOMIC_ACTION(kernel.sendShortAM_async(1, 4, H_INCR_BAR, 0, nullptr));
    TIMESTAMP(kern0_8)
    SAFE_COUT(COLOR(Color::FG_RED, dec, "kern0: sending short async\n"));

    kernel.wait_barrier(1);

    SAFE_COUT(COLOR(Color::FG_RED, dec, "Value at 0 is " << (char *)gasnet_shared_mem << "\n"));

    kernel.end();
    TIMESTAMP_INIT
    TIMESTAMP_DIFF(kern0_1, kern0_0, "Kern0 0 Create")
    TIMESTAMP_DIFF(kern0_2, kern0_1, "Kern0 1 Init")
    TIMESTAMP_DIFF(kern0_3, kern0_2, "Kern0 2 Attach")
    TIMESTAMP_DIFF(kern0_4, kern0_3, "Kern0 3 Barrier Wait")
    TIMESTAMP_DIFF(kern0_5, kern0_4, "Kern0 4 Copy Payload")
    TIMESTAMP_DIFF(kern0_6, kern0_5, "Kern0 5 Send Medium")
    TIMESTAMP_DIFF(kern0_7, kern0_6, "Kern0 6 Medium reply")
    TIMESTAMP_DIFF(kern0_8, kern0_7, "Kern0 7 Send Short Async")
    TIMESTAMP_END
}

// void kern1(
//     short id,
//     galapagos::interface<word_t> * in,
//     #ifdef __HLS__
//     galapagos::interface<word_t> * out,
//     int * handler_ctrl,
//     #else
//     galapagos::interface<word_t> * out
//     #endif
// ){
//     #pragma HLS INTERFACE axis port=in
//     #pragma HLS INTERFACE axis port=out
//     #pragma HLS INTERFACE ap_ctrl_none port=return
//     #pragma HLS INTERFACE m_axi port=handler_ctrl depth=32 offset=0

//     galapagos::stream_packet <word_t> axis_word;

//     SAFE_COUT("Entering kern1\n");

//     #ifdef __HLS__
//     shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out, handler_ctrl);
//     #else
//     shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out);
//     #endif

//     ATOMIC_ACTION(kernel.init());

//     #ifndef __HLS__
//     ATOMIC_ACTION(kernel.attach(handlers, 1, SEGMENT_SIZE));
//     #endif

//     TIMESTAMP(kern1_0)
//     kernel.barrier_send(KERN0_ID);
//     TIMESTAMP(kern1_1)
//     SAFE_COUT(COLOR(Color::FG_RED, dec, "kern1: sending barrier\n"));

//     while(in->empty()){};
//     TIMESTAMP(kern1_2)
//     axis_word = in->read();
//     TIMESTAMP(kern1_3)
//     #if ENABLE_PROFILE == 1
//     profile_read(axis_word);
//     axis_word = in->read();
//     profile_read(axis_word);
//     #else
//     axis_word = in->read();
//     #endif
//     #ifndef __HLS__
//     ATOMIC_ACTION(printWord("Data in kern1 arrived ", axis_word));
//     #endif
//     TIMESTAMP(kern1_4)
//     kernel.sendLongAM_normal(0, 0xF, 0, 0, nullptr, 8, &(axis_word.data), 0);
//     TIMESTAMP(kern1_5)
//     SAFE_COUT(COLOR(Color::FG_RED, dec, "kern1: sending long message\n"));

//     kernel.wait_reply(1); // from long message
//     TIMESTAMP(kern1_6)
//     SAFE_COUT(COLOR(Color::FG_RED, dec, "kern1: got reply\n"));

//     ATOMIC_ACTION(kernel.sendShortAM_async(0, 1, H_INCR_BAR, 0, nullptr));
//     SAFE_COUT(COLOR(Color::FG_RED, dec, "kern1: sending short async\n"));

//     kernel.wait_barrier(1);
//     SAFE_COUT(COLOR(Color::FG_RED, dec, "kern1: waiting barrier\n"));

//     kernel.end();
//     TIMESTAMP_INIT
//     TIMESTAMP_DIFF(kern1_1, kern1_0, "Kern1 0 Barrier Send")
//     TIMESTAMP_DIFF(kern1_2, kern1_1, "Kern1 1 Wait for Medium")
//     TIMESTAMP_DIFF(kern1_3, kern1_2, "Kern1 2 Read word")
//     // TIMESTAMP_DIFF(kern1_4, kern1_3, "None")
//     TIMESTAMP_DIFF(kern1_5, kern1_4, "Kern1 4 Send Long")
//     TIMESTAMP_DIFF(kern1_6, kern1_5, "Kern1 5 Long Reply")
//     TIMESTAMP_END
// }

#ifndef __HLS__
PGAS_METHOD(kern0, KERN0_ID)
// PGAS_METHOD(kern1, KERN1_ID)
#endif

}

#ifndef __HLS__
void S0_barrier_increment(gc_AMToken_t token){
    SAFE_COUT("In handler\n");
}
#endif
