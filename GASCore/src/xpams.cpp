#include "xpams.hpp"

void xpams(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_rx, //input from am_rx
    axis_t &axis_tx, //output to am_tx
    axis_dest_t &axis_kernel_out, //output to kernel
    axis_t &axis_kernel_in //input from kernel
){
    #pragma HLS INTERFACE axis port=axis_rx
    #pragma HLS INTERFACE axis port=axis_tx
    #pragma HLS INTERFACE axis port=axis_kernel_out
    #pragma HLS INTERFACE axis port=axis_kernel_in
	#pragma HLS INTERFACE ap_ctrl_none port=return

    #ifdef DEBUG
    #pragma HLS INTERFACE ap_none port=dbg_currentState
    #endif
    
    axis_word_t axis_word;
    axis_wordDest_t axis_wordDest;

    static gc_AMsrc_t AMsrc;
    static gc_AMdst_t AMdst;
    static gc_AMToken_t AMToken;

    static gc_payloadSize_t AMpayloadSize;

    static gc_AMargs_t AMargs;
    static gc_AMtype_t AMtype;
    static gc_AMhandler_t AMhandler;

    switch(currentState){
        case st_AMheader:{
            if(!axis_rx.empty()){
                axis_rx.read(axis_word);
                AMsrc = axis_word.data(15,0);
                AMdst = axis_word.data(31,16);
                AMpayloadSize = axis_word.data(43,32);
                AMhandler = axis_word.data(47,44);
                AMtype = axis_word.data(55,48);
                AMargs = axis_word.data(63,56);
                axis_rx.read(axis_word); //read token
                AMToken = axis_word.data(63,40);
                if (AMargs != 0){ //! temporary fix to just absorb all arguments
                    gc_AMargs_t i;
                    for(i = 0; i < AMargs; i++){
                        axis_rx.read(axis_word);
                    }
                }
                if(isReplyAM(AMtype)){
                    axis_word.data(63,40) = AMToken;
                    axis_wordDest = assignWord(axis_word);
                    axis_wordDest.dest = AMdst;
                    axis_kernel_out.write(axis_wordDest);
                    currentState = st_AMheader;
                }
                else{
                    if(isMediumAM(AMtype)){
                        currentState = st_AMpayload;
                    }
                    else{
                        currentState = st_sendReplyHeader;
                    }
                }
            }
            break;
        }
        case st_AMpayload:{
            gc_payloadSize_t i;
            for(i = 0; i < AMpayloadSize; i++){
                axis_rx.read(axis_word);
                axis_wordDest = assignWord(axis_word);
                axis_wordDest.dest = AMdst;
                axis_kernel_out.write(axis_wordDest);
            }
            currentState = st_sendReplyHeader;
            break;
        }
        case st_sendReplyHeader:{
            axis_word.data(15,0) = AMdst;
            axis_word.data(31,16) = AMsrc;
            axis_word.data(43,32) = 0;
            axis_word.data(47,44) = H_EMPTY;
            axis_word.data(55,48) = 0x41;
            axis_word.data(63,56) = 0;
            axis_word.keep = 0xFF; //! parameterize
            axis_tx.write(axis_word);
            axis_word.data(39,0) = 0;
            axis_word.data(63,40) = AMToken;
            axis_word.last = 1; //!needs to be handled better
            axis_word.keep = 0xFF; //! parameterize
            axis_tx.write(axis_word);
            currentState = st_AMheader;
            break;
        }
    }

    // if(!axis_kernel_in.empty() && currentState != st_sendReplyHeader){
    //     axis_kernel_in.read(axis_word);
    //     axis_tx.write(axis_word);
    // }

    #ifdef DEBUG
    dbg_currentState = currentState;
    #endif
}

inline axis_wordDest_t assignWord(axis_word_t axis_word){
    axis_wordDest_t axis_wordDest;

    axis_wordDest.data = axis_word.data;
    axis_wordDest.keep = axis_word.keep;
    axis_wordDest.last = axis_word.last;
    
    return axis_wordDest;
}

#ifdef DEBUG
std::string stateParse(int state){
    switch(state){
        // CHECK_STATE("st_idle", st_idle, 0)
        CHECK_STATE("st_AMheader", st_AMheader, 0)
        CHECK_STATE("st_sendReplyHeader", st_sendReplyHeader, 1)
        CHECK_STATE("st_AMpayload", st_AMpayload, 2)
        default: return "Unknown State";
    }
}
#endif