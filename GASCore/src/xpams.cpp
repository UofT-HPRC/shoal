#include "xpams.hpp"

void xpams(
    axis_t &axis_rx, //input
    axis_t &axis_handler_tx, //output
    axis_t &axis_kernel_out, //output
    axis_t &axis_kernel_in, //input
    axis_t &axis_kernel_tx, //output
    //connection to tx
    //connection to tx
    uint_1_t &blockingWait,

    counter_t AMcounter_threshold,
    counter_t wordCounter_threshold,
    counter_t customCounter_master[COUNTER_NUM],
    counter_t handlerCounter_threshold[COUNTER_NUM],
    counter_t handlerCounter_master[COUNTER_NUM],
    word_t enable,
    word_t mask
){
    #pragma HLS INTERFACE axis port=axis_rx
    #pragma HLS INTERFACE axis port=axis_handler_tx
    #pragma HLS INTERFACE axis port=axis_kernel_out
    #pragma HLS INTERFACE axis port=axis_kernel_in
    #pragma HLS INTERFACE axis port=axis_kernel_tx
	#pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_none port=blockingWait
    
    #pragma HLS INTERFACE s_axilite port=AMcounter_threshold bundle=control
    #pragma HLS INTERFACE s_axilite port=wordCounter_threshold bundle=control
    #pragma HLS INTERFACE s_axilite port=customCounter_master bundle=control
    #pragma HLS INTERFACE s_axilite port=handlerCounter_master bundle=control
    #pragma HLS INTERFACE s_axilite port=handlerCounter_threshold bundle=control
    #pragma HLS INTERFACE s_axilite port=enable bundle=control
    #pragma HLS INTERFACE s_axilite port=mask bundle=control

    axis_word_t axis_word;

    static enum state_t{st_idle, st_AMheader, st_increment, st_sendReplyHeader, 
        st_AMpayload, st_error} currentState;

    static gc_AMsrc_t AMsrc;
    static gc_AMdst_t AMdst;

    static counter_t AMcounter = 0;
    static counter_t wordCounter = 0;
    static counter_t customCounter[COUNTER_NUM];
    static counter_t handlerCounter[COUNTER_NUM];

    static gc_payloadSize_t AMpayloadSize;

    static gc_AMargs_t AMargs;
    static gc_AMtype_t AMtype;
    static gc_AMhandler_t AMhandler;

    switch(currentState){
        case st_idle:{
            if(enable[0] == 1){
                AMcounter = 0;
            }
            if(enable[1] == 1){
                wordCounter = 0;
            }
            int i;
            for(i=0; i < COUNTER_NUM; i++){
                if(enable[12+i] == 1){
                    customCounter[i] = 0;
                }
                if(enable[16+i] == 1){
                    handlerCounter[i] = 0;
                }
            }
            currentState == axis_rx.empty() ? st_idle : st_AMheader;
            break;
        }
        case st_AMheader:{
            if(!axis_rx.empty()){
                axis_rx.read(axis_word);
                AMsrc = axis_word.data(15,0);
                AMdst = axis_word.data(31,16);
                AMpayloadSize = axis_word.data(43,32);
                AMhandler = axis_word.data(47,44);
                AMtype = axis_word.data(55,48);
                AMargs = axis_word.data(63,56);
                AMhandler = axis_word.data(31,16);
                AMcounter++;
                if(enable[8] && handlerCounter_master[0](15,0) == AMhandler){
                    handlerCounter[0]++;
                }
                if(enable[9] && handlerCounter_master[1](15,0) == AMhandler){
                    handlerCounter[1]++;
                }
                if(enable[10] && handlerCounter_master[2](15,0) == AMhandler){
                    handlerCounter[2]++;
                }
                if(enable[11] && handlerCounter_master[3](15,0) == AMhandler){
                    handlerCounter[3]++;
                }
                if(isMediumAM(AMtype)){
                    currentState = st_AMpayload;
                }
                else{
                    switch(AMhandler){
                        case H_EMPTY:{
                            currentState = st_sendReplyHeader;
                            break;
                        }
                        case H_INCR:{
                            currentState = st_increment;
                            break;
                        }
                        default:{
                            currentState = st_error;
                            break;
                        }
                    }
                }
                break;
            }
        }
        case st_AMpayload:{
            gc_payloadSize_t i;
            for(i = 0; i < AMpayloadSize; i++){
                axis_rx.read(axis_word);
                axis_kernel_out.write(axis_word);
            }
            switch(AMhandler){
                case H_EMPTY:{
                    currentState = st_sendReplyHeader;
                    break;
                }
                case H_INCR:{
                    currentState = st_increment;
                    break;
                }
                default:{
                    currentState = st_error;
                    break;
                }
            }
        }
        case st_increment:{
            counterValue_t counterValue;
            counterIndex_t counterIndex;
            counter_t counterIncrement;
            axis_rx.read(axis_word);
            counterIndex = axis_word.data(1,0);
            counterValue = axis_word.data(3,2);
            switch(counterValue){
                case 0: counterIncrement = 2;
                case 1: counterIncrement = 1;
                case 2: counterIncrement = -2;
                case 3: counterIncrement = -1;
            }
            customCounter[counterIndex] += counterIncrement;
            if(isReplyAM(AMtype)){
                currentState = st_idle;
            }
            else{
                currentState = st_sendReplyHeader;
            }
        }
        case st_sendReplyHeader:{
            axis_word.data(15,0) = AMdst;
            axis_word.data(31,16) = AMsrc;
            axis_word.data(43,32) = 0;
            axis_word.data(47,44) = H_EMPTY;
            axis_word.data(55,48) = 0x41;
            axis_word.data(63,56) = 0;
            axis_handler_tx.write(axis_word);
            currentState = st_idle;
            break;
        }
        case st_error:{
            currentState = st_error;
        }
    }

    word_t wait_check = 
    ((enable[2] == 1 && AMcounter == AMcounter_threshold) << 2) |
    ((enable[3] == 1 && wordCounter == wordCounter_threshold) << 3) |
    ((enable[4] == 1 && customCounter[0] == customCounter_master[0]) << 4) |
    ((enable[5] == 1 && customCounter[1] == customCounter_master[1]) << 5) |
    ((enable[6] == 1 && customCounter[2] == customCounter_master[2]) << 6) |
    ((enable[7] == 1 && customCounter[3] == customCounter_master[3]) << 7) |
    ((enable[8] == 1 && handlerCounter[0] == handlerCounter_threshold[0]) << 8) |
    ((enable[9] == 1 && handlerCounter[1] == handlerCounter_threshold[1]) << 9) |
    ((enable[10] == 1 && handlerCounter[2] == handlerCounter_threshold[2]) << 10) |
    ((enable[11] == 1 && handlerCounter[3] == handlerCounter_threshold[3]) << 11);

    blockingWait = (enable != 0) && (wait_check(31,2) != enable(31,2));

    if(!axis_kernel_in.empty()){
        axis_kernel_in.read(axis_word);
        axis_kernel_out.write(axis_word);
    }
}