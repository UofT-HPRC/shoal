#include "one_node.hpp"

#define hc_S0_barrier_increment 7
void S0_barrier_increment(gc_AMToken_t token);

static gasnet_handlerentry_t handlers[] =
{
	{
		hc_S0_barrier_increment,
		(void(*)())S0_barrier_increment
	}
};

extern "C"{
void kern0(
    galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out
){
    int id = KERN0_ID;
    galapagos::stream_packet <word_t> axis_word;

    SAFE_COUT("Entering kern0\n");

    shoal::kernel kernel(id, KERNEL_NUM, in, out);

    ATOMIC_ACTION(kernel.init());
    ATOMIC_ACTION(kernel.attach(handlers, 1, SEGMENT_SIZE));

    kernel.barrier_wait();

    word_t payload;
    memcpy(&payload, "GAScore", 8);

    ATOMIC_ACTION(kernel.sendMediumAM_normal(1, 2, 0, 0, nullptr, 8, (word_t*)(&payload)));

    ATOMIC_ACTION(kernel.sendShortAM_async(1, 4, H_INCR_BAR, 0, nullptr));

    kernel.wait_reply(1); // from medium message

    kernel.wait_barrier(1);

    SAFE_COUT(COLOR(Color::FG_RED, dec, "Value at 0 is " << (char *)gasnet_shared_mem << "\n"));

    kernel.end();
}

void kern1(
    galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out
){
    int id = KERN1_ID;
    galapagos::stream_packet <word_t> axis_word;

    SAFE_COUT("Entering kern1\n");

    shoal::kernel kernel(id, KERNEL_NUM, in, out);

    ATOMIC_ACTION(kernel.init());
    ATOMIC_ACTION(kernel.attach(handlers, 1, SEGMENT_SIZE));

    kernel.barrier_send(KERN0_ID);

    while(in->empty()){};
    axis_word = in->read();
    axis_word = in->read();
    ATOMIC_ACTION(printWord("Data in kern1 arrived ", axis_word));
    kernel.sendLongAM_normal(0, 0xF, 0, 0, nullptr, 8, &(axis_word.data), 0);

    kernel.wait_reply(1); // from long message

    ATOMIC_ACTION(kernel.sendShortAM_async(0, 1, H_INCR_BAR, 0, nullptr));

    kernel.wait_barrier(1);

    kernel.end();
}

PGAS_METHOD(kern0, KERN0_ID)
PGAS_METHOD(kern1, KERN1_ID)
}

void S0_barrier_increment(gc_AMToken_t token){
    SAFE_COUT("In handler\n");
}
