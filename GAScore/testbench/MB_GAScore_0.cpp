#include <stdio.h>

#define __MICROBLAZE__
#define ENABLE_NETWORK
#define KERNEL_LINK 0
#define NET_LINK 1
#include "active_messages.hpp"
#include "register_map.hpp"

int main(){

    gc_AMsrc_t src = 0;
    gc_AMdst_t dst = 1;
    gc_payloadSize_t payloadSize = 2;
    gc_AMhandler_t handler = 0;
    gc_AMtype_t type = AM_MEDIUM|AM_FIFO;
    gc_AMargs_t handlerArgCount = 0;

    gc_AMToken_t token = 0;

    word_t handler_args [1];

    word_t payload [10] = {0,1,2,3,4,5,6,7,8,9};

    sendMediumAM(src, dst, token, handler, handlerArgCount, &handler_args[0], payloadSize, &payload[0]);

    dst = 16;
    sendMediumAM(src, dst, token, handler, handlerArgCount, &handler_args[0], payloadSize, &payload[0]);

    word_t word = readKernel();
    xil_printf("1: %d\n", word == 0); // token
    word = readKernel();
    xil_printf("2: %d\n", word == 0); // payload 0
    word = readKernel();
    xil_printf("3: %d\n", word == 1); // payload 1
    word = readKernel();
    xil_printf("4: %d\n", word == 65); // reply (0x41 and token of 0)

    bool empty = checkKernelEmpty();
	xil_printf("Kernel empty: %d\n", empty);
	empty = checkNetEmpty();
	xil_printf("Net empty: %d\n", empty);

    word = readNetwork();
    xil_printf("5: %d\n", word == 2199291691010); // header
	word = readNetwork();
	xil_printf("6: %d\n", word == 0); // token
	word = readNetwork();
	xil_printf("7: %d\n", word == 0); // payload 0
	word = readNetwork();
	xil_printf("8: %d\n", word == 1); // payload 1

	empty = checkKernelEmpty();
	xil_printf("Kernel empty: %d\n", empty);
	empty = checkNetEmpty();
	xil_printf("Net empty: %d\n", empty);

    xil_printf("Finished\n");

    return 0;

}
