#include "handler.hpp"

void handler(
    axis_noKeep_t &axis_handler, //input to handler
    gc_AMhandler_t AMhandler,
    uint_2_t config,
    uint_16_t config_handler,
    uint_32_t counter_threshold,
    uint_1_t &interrupt
){
    #pragma HLS INTERFACE axis port=axis_handler
	#pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_none port=interrupt
    #pragma HLS INTERFACE ap_none port=AMhandler
    #pragma HLS INTERFACE s_axilite port=config bundle=ctrl_bus
    #pragma HLS INTERFACE s_axilite port=config_handler bundle=ctrl_bus
    #pragma HLS INTERFACE s_axilite port=counter_threshold bundle=ctrl_bus

    #pragma HLS PIPELINE

    axis_wordNoKeep_t axis_word;
    static uint_32_t counter = 0;
    static uint_32_t barrier_cnt = 0;
    static uint_32_t mem_ready_barrier_cnt = 0;

    uint_1_t lock = config[0];
    uint_1_t reset = config[1];
    static uint_1_t interrupt_wire = 0;
    static uint_1_t acknowledged = 0;

    if(lock != 1){
        if(axis_handler.read_nb(axis_word)){
            switch(AMhandler){
                case H_INCR_MEM:{
                    mem_ready_barrier_cnt++;
                    break;
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
    } else {
        if (!acknowledged){
            switch(config_handler){
                case H_INCR_MEM:{
                    mem_ready_barrier_cnt -= counter_threshold;
                    break;
                }
                case H_ADD:{
                    counter -= counter_threshold;
                    break;
                }
                case H_INCR_BAR:{
                    barrier_cnt -= counter_threshold;
                    break;
                }
                default:
                    break;
            }
            acknowledged = 1;
        }
    }

    if (reset){
        interrupt_wire = 0;
        acknowledged = 0;
    } else {
        switch(config_handler){
            case H_INCR_MEM:{
                interrupt_wire = interrupt_wire | (mem_ready_barrier_cnt >= counter_threshold);
                break;
            }
            case H_ADD:{
                interrupt_wire = interrupt_wire | (counter >= counter_threshold);
                break;
            }
            case H_INCR_BAR:{
                interrupt_wire = interrupt_wire | (barrier_cnt >= counter_threshold);
                break;
            }
            default:
                break;
        }
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
