#include "handler.hpp"

void handler(
    axis_noKeep_t &axis_handler, //input to handler
    gc_AMhandler_t AMhandler,
    uint_3_t config,
    uint_32_t counter_threshold,
    uint_32_t token,
    uint_1_t &interrupt
){
    #pragma HLS INTERFACE axis port=axis_handler
	#pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_none port=interrupt
    #pragma HLS INTERFACE ap_none port=AMhandler
    #pragma HLS INTERFACE s_axilite port=config bundle=ctrl_bus
    #pragma HLS INTERFACE s_axilite port=counter_threshold bundle=ctrl_bus

    axis_wordNoKeep_t axis_word;
    static uint_32_t counter = 0;
    static uint_32_t barrier_cnt = 0;
    static uint_32_t mem_ready_barrier_cnt = 0;

    uint_1_t reset = config[0];
    uint_1_t enable_add = config[1];
    static uint_1_t interrupt_wire = 0;
    
    if(reset != 1){
        if(axis_handler.read_nb(axis_word)){
            switch(AMhandler){
                case H_INCR_MEM:{
                    mem_ready_barrier_cnt++;
                }
                case H_ADD:{
                    counter += axis_word.data(31,0);
                    break;
                }
                case H_INCR_BAR:{
                    barrier_cnt++;
                    break;
                }
                default:{
                    break;
                }
            }
        }
    }

    if (config & 1){
        interrupt_wire = 0;
    } else if(config == H_INCR_MEM<<1){
        interrupt_wire = interrupt_wire | (mem_ready_barrier_cnt >= counter_threshold);
    } else if(config == H_ADD<<1){
        interrupt_wire = interrupt_wire | (counter >= counter_threshold);
    } else if(config == H_INCR_BAR<<1){
        interrupt_wire = interrupt_wire | (barrier_cnt >= counter_threshold);
    }

    interrupt = interrupt_wire;
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
