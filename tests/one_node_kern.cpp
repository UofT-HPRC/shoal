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

void replyWait(int id, galapagos::stream <word_t> * in){
    galapagos::stream_packet <word_t> axis_word;
    std::stringstream ss;
    ss << "Reply arrived in kernel " << id << " - ";
    while(!in->empty()){
        axis_word = in->read();
        printWord(ss.str(), axis_word);
    }
}

extern "C"{
void kern0(
    galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out
){
    int id = KERN0_ID;
    galapagos::stream_packet <word_t> axis_word;

    // void* shared_mem;
    SAFE_COUT("Entering kern0\n");

    gasnet_attach(handlers, 1);

    gasnet_shared_mem = gasnet_init(id);
    nodedata = &(gasnet_nodedata_all[id]);

    while(nodedata->mem_ready_barrier_cnt != 1){sched_yield();};
    ATOMIC_ACTION(sendShortAM(id, 1, 4, H_INCR_MEM, 0, nullptr, *out));

    while(in->empty()){};
    while(!in->empty()){
        axis_word = in->read();
        printWord("Reply arrived in kernel 0 ", axis_word);
    };

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

    while(in->empty()){};
    while(!in->empty()){
        axis_word = in->read();
        printWord("Reply arrived in kernel 0 ", axis_word);
    };

    ATOMIC_ACTION(sendShortAM(id, 1, 4, H_INCR_BAR, 0, nullptr, *out));

    while(in->empty()){};
    ATOMIC_ACTION(replyWait(id, in));

    while(nodedata->barrier_cnt != 1){sched_yield();};

    while(!(in->empty())){
        axis_word = in->read();
        ATOMIC_ACTION(printWord("Reading overflow data on kern0 ", axis_word));
    }

    SAFE_COUT("Leaving kern0\n");

    *(kernel_done[id]) = true;
}

void kern1(
    galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out
){
    int id = KERN1_ID;
    galapagos::stream_packet <word_t> axis_word;

    SAFE_COUT("Entering kern1\n");

    gasnet_attach(handlers, 1);

    gasnet_shared_mem = gasnet_init(id);
    nodedata = &(gasnet_nodedata_all[id]);

    ATOMIC_ACTION(sendShortAM(id, 0, 3, H_INCR_MEM, 0, nullptr, *out));
    while(nodedata->mem_ready_barrier_cnt != 1){sched_yield();};

    while(in->empty()){};
    ATOMIC_ACTION(replyWait(id, in));

    while(in->empty()){};
    while(!(in->empty())){
        axis_word = in->read();
        ATOMIC_ACTION(printWord("Data in kern1 arrived ", axis_word));
    }

    ATOMIC_ACTION(sendShortAM(id, 0, 4, H_INCR_BAR, 0, nullptr, *out));

    while(nodedata->barrier_cnt != 1){sched_yield();};

    while(in->empty()){};
    ATOMIC_ACTION(replyWait(id, in));

    while(!(in->empty())){
        axis_word = in->read();
        ATOMIC_ACTION(printWord("Reading overflow data on kern1 ", axis_word));
    }

    SAFE_COUT("Leaving kern1\n");

    *kernel_done[id] = true;
}

PGAS_METHOD(kern0, KERN0_ID)
PGAS_METHOD(kern1, KERN1_ID)
}

void S0_barrier_increment(gc_AMToken_t token){
    SAFE_COUT("In handler\n");
}
