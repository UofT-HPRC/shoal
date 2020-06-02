#include "ap_int.h"
#include "hls_stream.h"

struct axis_struct {
    ap_uint<64> data;
    ap_uint<8> keep;
    ap_uint<1> last;
};

typedef axis_struct axis_word_t;
typedef hls::stream<axis_word_t> axis_t;

enum state_t{st_header, st_token, st_step, st_done};

// static state_t currentState = st_header;
static ap_uint<8> AMtype = -1;

state_t func_header(axis_t &axis_in, axis_t &axis_out){
    #pragma HLS PIPELINE
    axis_word_t axis_word;
    axis_in.read(axis_word);
    AMtype = axis_word.data(7,0);
    axis_out.write(axis_word);
    if(AMtype == 4){
        return st_done;
    } else {
        return st_token;
    }
}

state_t func_token(axis_t &axis_in, axis_t &axis_out){
    #pragma HLS PIPELINE
    axis_word_t axis_word;
    axis_in.read(axis_word);
    axis_out.write(axis_word);
    if(AMtype == 6){
        return st_step;
    } else {
        return st_done;
    }
}

state_t func_step(axis_t &axis_in, axis_t &axis_out){
    #pragma HLS PIPELINE
    axis_word_t axis_word;
    axis_in.read(axis_word);
    axis_out.write(axis_word);
    return st_done;
}

state_t func_done(axis_t &axis_in, axis_t &axis_out){
    #pragma HLS PIPELINE
    axis_word_t axis_word;
    axis_in.read(axis_word);
    axis_out.write(axis_word);
    if(axis_word.last){
        return st_header;
    }
    return st_done;
}

void test(
    axis_t &axis_in,
    axis_t &axis_out,
    state_t currentState,
    state_t& nextState
){
    #pragma HLS INTERFACE axis port=axis_in
    #pragma HLS INTERFACE axis port=axis_out
    #pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_none port=currentState
    #pragma HLS INTERFACE ap_vld port=nextState

    #pragma HLS PIPELINE

    // static state_t currentState = st_header;
    
    static ap_uint<8> AMtype = -1;
    
    // #pragma HLS DEPENDENCE variable=currentState intra WAR false

    switch(currentState){
        case st_header:{
            nextState = func_header(axis_in, axis_out);
            // axis_word_t axis_word;
            // axis_in.read(axis_word);
            // AMtype = axis_word.data(7,0);
            // axis_word.keep = 1;
            // axis_out.write(axis_word);
            // if(AMtype == 4){
            //     nextState = st_done;
            // } else {
            //     nextState = st_token;
            // }
            break;
        }
        case st_token:{
            nextState = func_token(axis_in, axis_out);
            // axis_word_t axis_word;
            // axis_in.read(axis_word);
            // axis_word.keep = 2;
            // axis_out.write(axis_word);
            // if(AMtype == 6){
            //     nextState = st_step;
            // } else {
            //     nextState = st_done;
            // }
            break;
        }
        case st_step:{
            nextState = func_step(axis_in, axis_out);
            // axis_word_t axis_word;
            // axis_in.read(axis_word);
            // axis_word.keep = 3;
            // axis_out.write(axis_word);
            // nextState = st_done;
            break;
        }
        case st_done:{
            nextState = func_done(axis_in, axis_out);
            // axis_word_t axis_word;
            // axis_in.read(axis_word);
            // axis_word.keep = 4;
            // axis_out.write(axis_word);
            // if(axis_word.last){
            //     nextState = st_header;
            // } else {
            //     nextState = st_done;
            // }
            break;
        }

    }

    // currentState = nextState;


}
