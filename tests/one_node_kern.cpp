#include "active_messages.hpp"

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

extern "C"{
void kern0(
    galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out
){
    std::cout << "Entering kern0\n";

    gasnet_attach(handlers, 1);

    gasnet_shared_mem[0] = (char*) malloc(1024);

    std::cout << "Kern 0 mem: " << std::hex << gasnet_shared_mem[0] << "\n";

    word_t payload;
    memcpy(&payload, "GAScore", 8);

    gc_AMsrc_t src = 0;
    gc_AMdst_t dst = 1;
    gc_AMToken_t token = 2;
    gc_AMhandler_t handlerID = 0;
    gc_AMargs_t handlerArgCount = 0;
    word_t * handler_args = NULL;
    gc_payloadSize_t payloadSize = 8;
    sendMediumAM(src, dst, token, handlerID, handlerArgCount, 
        handler_args, payloadSize, (word_t*)(&payload), *out);

    std::cout << "Leaving kern0\n";

    *(kernel_done[0]) = true;
}

void kern1(
    galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out
){
    std::cout << "Entering kern1\n";

    gasnet_attach(handlers, 1);

    gasnet_shared_mem[0] = (char*) malloc(1024);

    std::cout << "Kern 1 mem: " << std::hex << gasnet_shared_mem[0] << "\n";

    while(in->empty()){};
    std::cout << "Data in kern1 arrived\n";

    galapagos::stream_packet <word_t> axis_word;

    while(!(in->empty())){
        axis_word = in->read();
        std::cout << "Data is " << std::hex << axis_word.data << "\n";
        std::cout << "  Last is " << std::hex << axis_word.last << "\n";
        std::cout << "  Keep is " << std::hex << axis_word.keep << "\n";
    }

    std::cout << "Leaving kern1\n";

    *kernel_done[1] = true;
}

PGAS_METHOD(kern0, 0)
PGAS_METHOD(kern1, 1)
}

void S0_barrier_increment(gc_AMToken_t token){
    std::cout << "In handler\n";
}
