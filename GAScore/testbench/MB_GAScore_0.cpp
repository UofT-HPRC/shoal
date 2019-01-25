#include <stdio.h>

#define __MB__
#define KERNEL_LINK 0
#define NET_LINK 1
#include "active_messages.hpp"

int main(){

    int src = 0;
    int dst = 1;
    int payloadSize = 2;
    int handler = 0;
    int type = AM_MEDIUM|AM_FIFO;
    int handlerArgCount = 0;

    int token = 0;

    int handler_args [1];

    long long payload [10] = {0,1,2,3,4,5,6,7,8,9};

    sendMediumAM(src, dst, token, handler, handlerArgCount, &handler_args, payloadSize, &payload);

    dst = 16;
    sendMediumAM(src, dst, token, handler, handlerArgCount, &handler_args, payloadSize, &payload);

    long long word = readKernel();
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
