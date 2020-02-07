#include "handler.hpp"

void handler(
    axis_noKeep_t &axis_handler, //input to handler
    gc_AMhandler_t AMhandler,
    uint_32_t config,
    ap_int<32> arg,
    ap_int<32> &counter_out,
    ap_int<32> &barrier_out,
    ap_int<32> &mem_ready_out
){
    #pragma HLS INTERFACE axis port=axis_handler
	#pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_none port=AMhandler
    #pragma HLS INTERFACE s_axilite port=config bundle=ctrl_bus
    #pragma HLS INTERFACE s_axilite port=arg bundle=ctrl_bus
    #pragma HLS INTERFACE s_axilite port=counter_out bundle=ctrl_bus
    #pragma HLS INTERFACE s_axilite port=barrier_out bundle=ctrl_bus
    #pragma HLS INTERFACE s_axilite port=mem_ready_out bundle=ctrl_bus

    // #pragma HLS INTERFACE ap_none port=counter_out
    // #pragma HLS INTERFACE ap_none port=barrier_out
    // #pragma HLS INTERFACE ap_none port=mem_ready_out

    // #pragma HLS PIPELINE
    // #pragma HLS DATAFLOW

    axis_wordNoKeep_t axis_word;
    static ap_int<32> counter;
    static ap_int<32> barrier_cnt;
    static ap_int<32> mem_ready_barrier_cnt;
    static bool nonce = false;

    volatile bool lock = config.get_bit(4) == 0;

    counter_out = counter;
    barrier_out = barrier_cnt;
    mem_ready_out = mem_ready_barrier_cnt;
    
    if(lock || nonce){
        if (axis_handler.read_nb(axis_word)){
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
        if (lock){
            nonce = false;
        }
    } else {
        switch(config.range(3,0)){
            case H_INCR_MEM:{
                mem_ready_barrier_cnt -= arg;
                break;
            }
            case H_ADD:{
                counter -= arg;
                break;
            }
            case H_INCR_BAR:{
                barrier_cnt -= arg;
                break;
            }
            default:{
                break;
            }
        }
        // config = config & 0xF; // clear out the lock
        nonce = true;
    }

}
