#include "handler.hpp"

void handler(
    axis_noKeep_t &axis_handler, //input to handler
    gc_AMhandler_t AMhandler,
    uint_32_t counter_threshold,
    uint_2_t counter_reset,
    uint_1_t &interrupt
){
    #pragma HLS INTERFACE axis port=axis_handler
	#pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_none port=interrupt
    #pragma HLS INTERFACE ap_none port=AMhandler
    #pragma HLS INTERFACE s_axilite port=counter_threshold bundle=ctrl_bus
    #pragma HLS INTERFACE s_axilite port=counter_reset bundle=ctrl_bus

    axis_wordNoKeep_t axis_word;
    static uint_32_t counter = 0;

    uint_1_t enable = counter_reset[0];
    uint_1_t reset = counter_reset[1];
    
    if(axis_handler.read_nb(axis_word)){
        switch(AMhandler){
            case H_ADD:{
                counter += + axis_word.data(31,0);
                break;
            }
            default:{
                break;
            }
        }
    }
    else{
        if(reset == 1){
            counter = 0;
        }
    }
    
    if ((enable == 1) && (counter > counter_threshold)){
        interrupt = 1;
    }
    else{
        interrupt = 0;
    }
}

#ifdef DEBUG
std::string stateParse(int state){
    switch(state){
        CHECK_STATE("st_AMHeader", st_header, 0)
        CHECK_STATE("st_handler", st_AMHandlerArgs, 1)
        default: return "Unknown State";
    }
}
#endif