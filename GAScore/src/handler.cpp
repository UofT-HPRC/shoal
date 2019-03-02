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
    #pragma HLS INTERFACE s_axilite port=token bundle=ctrl_bus

    axis_wordNoKeep_t axis_word;
    static uint_32_t counter = 0;

    uint_1_t reset = config[0];
    uint_1_t enable_add = config[1];
    uint_1_t enable_token = config[2];

    static uint_32_t prev_tokens [PREV_TOKENS] = {0,0,0};
    // #pragma HLS ARRAY_PARTITION variable=prev_tokens complete dim=0
    
    if(axis_handler.read_nb(axis_word)){
        switch(AMhandler){
            case H_ADD:{
                counter += + axis_word.data(31,0);
                break;
            }
            case H_WAIT:{
                for(int i = 0; i < PREV_TOKENS - 1; i++){
                    // #pragma HLS unroll
                    prev_tokens[i] = prev_tokens[i+1];
                }
                prev_tokens[PREV_TOKENS-1] = axis_word.data(31,0);
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
            for(int i = 0; i < PREV_TOKENS; i++){
                // #pragma HLS unroll
                prev_tokens[i] = 0;
            }
        }
    }
    
    switch (config){
        case H_ADD:{
            interrupt = counter > counter_threshold;
            break;
        }
        case H_WAIT:{
            interrupt = prev_tokens[0] == token || prev_tokens[1] == token || 
                prev_tokens[2] == token;
            break;
        }
        default:
            interrupt = 0;
            break;
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
