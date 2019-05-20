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
	int interrupt;
	int init_time, message_time, final_time;
	word_t payload [10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	word_t handlerArg = 1;

    // initialize interrupt threshold
	writeReg(GASCORE_MEMORY_ADDR, COUNTER_THRESHOLD_ADDR, 1);
	writeReg(GASCORE_MEMORY_ADDR, COUNTER_ENABLE_ADDR, COUNTER_ENABLE);

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
	
    // wait for interrupt (set by short message from MB0)
    interrupt = 0;
	while(interrupt != 1){
		interrupt = (XGpio_DiscreteRead(&Gpio, 1)) & 1;
	}

    // send one word back to indicate interrupt worked
	sendMediumAM(16, 0, 11, 0, 0, &handlerArg, 1, (word_t)0x10);

	word_t reply = readKernel();
	xil_printf("MB1: Lower: %ld, Upper: %ld\n", (int)(reply & 0xFFFFFFFF), (int)((reply>>32) & 0xFFFFFFFF));

	xil_printf("MB1 Finished, empty: %ld\n", checkKernelEmpty());

	return 0;
}
