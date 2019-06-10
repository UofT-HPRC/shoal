#include "one_node.hpp"

// #define hc_S3_receive_delay 5
// void S3_receive_delay(gc_AMToken_t token, gc_AMargs_t arg0, gc_AMargs_t arg1, gc_AMargs_t arg2);

// #define hc_L0_receivestring 6
// void L0_receivestring(gc_AMToken_t token, void *buf, size_t nbytes);

#define hc_S0_barrier_increment 7
void S0_barrier_increment(gc_AMToken_t token);

static gasnet_handlerentry_t handlers[] =
{
	// {
	// 	hc_S3_receive_delay,
	// 	(void(*)())S3_receive_delay
	// },
	// {
	// 	hc_L0_receivestring,
	// 	(void(*)())L0_receivestring
	// },
	{
		hc_S0_barrier_increment,
		(void(*)())S0_barrier_increment
	}
};

// void replyWait(int id, galapagos::stream <word_t> * in){
//     galapagos::stream_packet <word_t> axis_word;
//     std::stringstream ss;
//     ss << "Reply arrived in kernel " << id << " - ";
//     while(!in->empty()){
//         axis_word = in->read();
//         printWord(ss.str(), axis_word);
//     }
// }

extern "C"{
void kern0(
    galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out
){
    int id = KERN0_ID;
    galapagos::stream_packet <word_t> axis_word;

    // void* shared_mem;
    SAFE_COUT("Entering kern0\n");

    shoal::kernel kernel(id, KERNEL_NUM, in, out);

    // gasnet_init(id);
    ATOMIC_ACTION(kernel.init());
    // gasnet_attach(handlers, 1, id, SEGMENT_SIZE);
    ATOMIC_ACTION(kernel.attach(handlers, 1, SEGMENT_SIZE));

    // while(nodedata->mem_ready_barrier_cnt != 1){sched_yield();};
    // kernel.wait_mem(1);
    // ATOMIC_ACTION(sendShortAM(id, 1, 4, H_INCR_MEM, 0, nullptr, *out));
    // ATOMIC_ACTION(kernel.sendMemUpdate(KERN1_ID));
    // ATOMIC_ACTION(kernel.barrier_wait());
    kernel.barrier_wait();

    // while(in->empty()){};
    // while(!in->empty()){
    //     axis_word = in->read();
    //     printWord("Reply arrived in kernel 0 ", axis_word);
    // };
    // kernel.wait_reply(1);

    word_t payload;
    memcpy(&payload, "GAScore", 8);

    gc_AMsrc_t src = id;
    gc_AMdst_t dst = 1;
    gc_AMToken_t token = 2;
    gc_AMhandler_t handlerID = 0;
    gc_AMargs_t handlerArgCount = 0;
    word_t * handler_args = nullptr;
    gc_payloadSize_t payloadSize = 8;
    ATOMIC_ACTION(sendMediumAM(src, dst, token, handlerID, handlerArgCount, 
        handler_args, payloadSize, (word_t*)(&payload), *out));

    // while(in->empty()){};
    // while(!in->empty()){
    //     axis_word = in->read();
    //     printWord("Reply arrived in kernel 0 ", axis_word);
    // };

    ATOMIC_ACTION(sendShortAM(AM_SHORT, id, 1, 4, H_INCR_BAR, 0, nullptr, *out));

    // while(in->empty()){};
    // ATOMIC_ACTION(replyWait(id, in));
    kernel.wait_reply(2);

    // while(nodedata->barrier_cnt != 1){sched_yield();};
    kernel.wait_barrier(1);

    while(!(in->empty())){
        axis_word = in->read();
        ATOMIC_ACTION(printWord("Reading overflow data on kern0 ", axis_word));
    }

    // SAFE_COUT("Leaving kern0\n");

    // free(gasnet_shared_mem_global[id]);
    // *(kernel_done[id]) = true;
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

    // gasnet_init(id);
    ATOMIC_ACTION(kernel.init());
    // gasnet_attach(handlers, 1, id, SEGMENT_SIZE);
    ATOMIC_ACTION(kernel.attach(handlers, 1, SEGMENT_SIZE));

    // ATOMIC_ACTION(sendShortAM(id, 0, 3, H_INCR_MEM, 0, nullptr, *out));
    // ATOMIC_ACTION(kernel.sendMemUpdate(KERN0_ID));
    // while(nodedata->mem_ready_barrier_cnt != 1){sched_yield();};
    // kernel.wait_mem(1);
    // ATOMIC_ACTION(kernel.barrier_send(KERN0_ID));
    kernel.barrier_send(KERN0_ID);

    // while(in->empty()){};
    // ATOMIC_ACTION(replyWait(id, in));
    // kernel.wait_reply(1);

    while(in->empty()){};
    while(!(in->empty())){
        axis_word = in->read();
        ATOMIC_ACTION(printWord("Data in kern1 arrived ", axis_word));
    }

    ATOMIC_ACTION(sendShortAM(AM_SHORT, id, 0, 4, H_INCR_BAR, 0, nullptr, *out));

    // while(nodedata->barrier_cnt != 1){sched_yield();};
    kernel.wait_barrier(1);

    // while(in->empty()){};
    // ATOMIC_ACTION(replyWait(id, in));
    kernel.wait_reply(1);

    while(!(in->empty())){
        axis_word = in->read();
        ATOMIC_ACTION(printWord("Reading overflow data on kern1 ", axis_word));
    }

    // SAFE_COUT("Leaving kern1\n");

    // free(gasnet_shared_mem_global[id]);
    // *kernel_done[id] = true;
    kernel.end();
}

PGAS_METHOD(kern0, KERN0_ID)
PGAS_METHOD(kern1, KERN1_ID)
}

void S0_barrier_increment(gc_AMToken_t token){
    SAFE_COUT("In handler\n");
}
