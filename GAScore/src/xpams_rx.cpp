#include "xpams_rx.hpp"

void xpams_rx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_rx, //input from am_rx
    axis_t &axis_tx, //output to am_tx (auto reply)
    axis_noKeep_t &axis_handler, //output to handler
    axis_dest_t &axis_kernel_out //output to kernel
){
    #pragma HLS INTERFACE axis port=axis_rx
    #pragma HLS INTERFACE axis port=axis_tx
    #pragma HLS INTERFACE axis port=axis_handler
    #pragma HLS INTERFACE axis port=axis_kernel_out
	#pragma HLS INTERFACE ap_ctrl_none port=return

    #ifdef DEBUG
    #pragma HLS INTERFACE ap_none port=dbg_currentState
    #endif
    
    axis_word_t axis_word;
    axis_wordDest_t axis_wordDest;
    axis_wordNoKeep_t axis_wordNoKeep;

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
                AMsrc = axis_word.data(AM_SRC);
                AMdst = axis_word.data(AM_DST);
                AMpayloadSize = axis_word.data(AM_PAYLOAD_SIZE);
                AMhandler = axis_word.data(AM_HANDLER);
                AMtype = axis_word.data(AM_TYPE);
                AMargs = axis_word.data(AM_HANDLER_ARGS);
                if (AMhandler != H_EMPTY){
                    axis_wordNoKeep = assignWordtoNoKeep(axis_word);
                    axis_handler.write(axis_wordNoKeep);
                }
                axis_rx.read(axis_word); //read token
                AMToken = axis_word.data(AM_TOKEN);
                if (AMargs != 0){
                    gc_AMargs_t i;
                    for(i = 0; i < AMargs - 1; i++){
                        axis_rx.read(axis_word);
                        axis_wordNoKeep = assignWordtoNoKeep(axis_word);
                        axis_handler.write(axis_wordNoKeep);
                    }
                    axis_rx.read(axis_word);
                    axis_wordNoKeep = assignWordtoNoKeep(axis_word);
                    axis_wordNoKeep.last = 1;
                    axis_handler.write(axis_wordNoKeep);
                }
                if(isReplyAM(AMtype)){
                    axis_word.data(AM_TOKEN) = AMToken;
                    axis_word.data(39,8) = 0;
                    axis_word.data(AM_TYPE) = AM_SHORT + AM_REPLY;
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
            axis_word.data(AM_SRC) = AMdst;
            axis_word.data(AM_DST) = AMsrc;
            axis_word.data(AM_PAYLOAD_SIZE) = 0;
            axis_word.data(AM_HANDLER) = H_EMPTY;
            axis_word.data(AM_TYPE) = AM_SHORT + AM_REPLY;
            axis_word.data(AM_HANDLER_ARGS) = 0;
            axis_word.keep = GC_DATA_TKEEP;
            axis_tx.write(axis_word);
            axis_word.data(39,0) = 0;
            axis_word.data(AM_TOKEN) = AMToken;
            // axis_word.data(39,) = 0;
            axis_word.last = 1; //!needs to be handled better
            axis_word.keep = GC_DATA_TKEEP;
            axis_tx.write(axis_word);
            currentState = st_AMheader;
            break;
        }
    }

    #ifdef DEBUG
    dbg_currentState = currentState;
    #endif
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