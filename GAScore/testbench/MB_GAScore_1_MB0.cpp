#define KERNEL_LINK 0
#define __MICROBLAZE__

#include "xparameters.h"
#include "xgpio.h"
#include "xtmrctr.h"
#include "xil_printf.h"

#include "active_messages.hpp"
#include "register_map.hpp"

#define TmrCtrNumber 0
#define GASCORE_MEMORY_ADDR 0x44a00000

XGpio Gpio;
XTmrCtr TimerCounter;

int main(){
	int Status;
	int init_time, message_time, final_time;
	word_t payload [10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	word_t handlerArg = 2;

	/* Initialize the GPIO driver */
	Status = XGpio_Initialize(&Gpio, XPAR_GPIO_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("GPIO Initialization Failed\r\n");
		return XST_FAILURE;
	}
	XGpio_SetDataDirection(&Gpio, 1, 0xFFFFFFFF); //set all to inputs on channel 1

	/* Initialize the Timer */
	Status = XTmrCtr_Initialize(&TimerCounter, XPAR_TMRCTR_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	Status = XTmrCtr_SelfTest(&TimerCounter, TmrCtrNumber);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XTmrCtr_SetOptions(&TimerCounter, TmrCtrNumber, XTC_AUTO_RELOAD_OPTION);

	init_time = XTmrCtr_GetValue(&TimerCounter, TmrCtrNumber);

	XTmrCtr_Start(&TimerCounter, TmrCtrNumber);

	sendLongAM(0, 16, 9, 0, 0, &handlerArg, 10, &payload[0], 0);

	message_time = XTmrCtr_GetValue(&TimerCounter, TmrCtrNumber);

	long long reply = readKernel(); // reply for long message
	xil_printf("MB0: Lower: %ld, Upper: %ld\n", (int)(reply & 0xFFFFFFFF), (int)((reply>>32) & 0xFFFFFFFF));

	sendShortAM(0, 16, 10, 1, 1, &handlerArg);
	reply = readKernel(); // reply for short message
	xil_printf("MB0: Lower: %ld, Upper: %ld\n", (int)(reply & 0xFFFFFFFF), (int)((reply>>32) & 0xFFFFFFFF));

    // delay loop to avoid concurrent xil_prints from both MBs
	int j;
	for (int i = 0; i < 1000000; i++){
		j++;
	}

	reply = readKernel(); // read word from medium message from MB1
	xil_printf("MB0: Lower: %ld, Upper: %ld\n", (int)(reply & 0xFFFFFFFF), (int)((reply>>32) & 0xFFFFFFFF));

	final_time = XTmrCtr_GetValue(&TimerCounter, TmrCtrNumber);

	xil_printf("MB0: Message time: %ld\n", message_time - init_time);
	xil_printf("MB0: Final time: %ld\n", final_time - message_time);

    // j is printed so it's not optimized away
	xil_printf("MB0 Finished %ld, Empty: %ld\n", j, checkKernelEmpty());

	return 0;
}
