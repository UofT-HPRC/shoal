#include "xpams_tx.hpp"

void xpams_tx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_kernel_in, //input from kernel
    axis_dest_t &axis_kernel_out, //output to kernel
    axis_t &axis_tx, // output to am_tx (from kernel)

    axis_noKeep_t &axis_handler, //output to handler

    const gc_AMdest_t address_offset_low,
    const gc_AMdest_t address_offset_high
    // const ap_uint<NETWORK_HEADER_LENGTH> network_table[256],
    // const ap_uint <NETWORK_HEADER_LENGTH> network_addr
){
    #pragma HLS INTERFACE axis port=axis_kernel_in
    #pragma HLS INTERFACE axis port=axis_tx
    #pragma HLS INTERFACE axis port=axis_kernel_out
    #pragma HLS INTERFACE axis port=axis_handler
	#pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_stable port=address_offset_high
    #pragma HLS INTERFACE ap_stable port=address_offset_low
    // #pragma HLS INTERFACE bram port=network_table
    // #pragma HLS INTERFACE ap_stable port=network_addr

    #pragma HLS DATAFLOW
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

    bool loopback;
    ap_uint <NETWORK_HEADER_LENGTH> network_addr_in;

    switch(currentState){
        case st_AMheader:{
            axis_kernel_in.read(axis_word);
            AMsrc = axis_word.data(AM_SRC);
            AMhandler = axis_word.data(AM_HANDLER);
            AMargs = axis_word.data(AM_HANDLER_ARGS);
            AMdst = axis_word.data(AM_DST);
            AMtype = axis_word.data(AM_TYPE);
            AMpayloadSize = axis_word.data(AM_PAYLOAD_SIZE);
            loopback = AMdst <= address_offset_high && AMdst >= address_offset_low && 
                !isLongxAM(AMtype) && !(isMediumAM(AMtype) && !isDataFromFIFO(AMtype));
            // network_addr_in = network_table[AMdst];
            // loopback = (network_addr == network_addr_in);
            if (loopback){
                // if (AMhandler != H_EMPTY){
                //     axis_wordNoKeep = assignWordtoNoKeep(axis_word);
                //     axis_wordNoKeep.last = AMargs == 0;
                //     axis_handler.write(axis_wordNoKeep);
                // }
                currentState = st_AMloopback;
            }
            else{
                axis_tx.write(axis_word);
                currentState = st_AMsend;
            }
            break;
        }
        // case st_AMtoken:{
        //     axis_kernel_in.read(axis_word); //read token
        //     AMToken = axis_word.data(AM_TOKEN);
        //     axis_tx.write(axis_word);
        //     currentState = st_AMsend;
        //     break;
        // }
        case st_AMloopback:{
            axis_kernel_in.read(axis_word); //read token
            AMToken = axis_word.data(AM_TOKEN);
            if (AMhandler != H_EMPTY){
                axis_wordNoKeep = createHandlerHeader(AMtype, AMToken, AMdst,
                    AMpayloadSize, AMhandler, AMargs);
                axis_handler.write(axis_wordNoKeep);
            }
            if (isMediumAM(AMtype)){
                #ifdef USE_ABS_PAYLOAD
                gc_payloadSize_t payload = AMpayloadSize - AMargs - GC_DATA_BYTES; // ! assuming payloadsize == dst size
                #else
                gc_payloadSize_t payload = AMpayloadSize; // ! assuming payloadsize == dst size
                #endif
                axis_wordDest = createKernelHeader(AMtype, AMToken, AMsrc, AMdst,
                    payload, AMhandler, AMargs);
                // axis_word.data(AM_TYPE) = AMtype;
                // axis_word.data(AM_SRC) = AMsrc;
                // #ifdef USE_ABS_PAYLOAD
                // axis_word.data(AM_DST) = AMpayloadSize - AMargs - GC_DATA_BYTES; // ! assuming payloadsize == dst size
                // #else
                // axis_word.data(AM_PAYLOAD_SIZE) = AMpayloadSize; // ! assuming payloadsize == dst size
                // #endif
                // axis_word.data(AM_TOKEN) = AMToken;
                // axis_wordDest = createKernelHeader(axis_word);
                // axis_wordDest.dest = AMdst;
                axis_kernel_out.write(axis_wordDest);
                currentState = AMargs != 0 ? st_AMargs : st_AMpayload;
            }
            else {
                currentState = AMargs != 0 ? st_AMargs : st_AMreply;
            }
            break;
        }
        case st_AMargs:{
            gc_AMargs_t i;
            for(i = 0; i < AMargs - 1; i++){
                #pragma HLS loop_tripcount min=1 max=255 avg=2
                #pragma HLS PIPELINE
                axis_kernel_in.read(axis_word);
                axis_wordNoKeep = assignWordtoNoKeep(axis_word);
                axis_handler.write(axis_wordNoKeep);
            }
            axis_kernel_in.read(axis_word);
            axis_wordNoKeep = assignWordtoNoKeep(axis_word);
            axis_wordNoKeep.last = 1;
            axis_handler.write(axis_wordNoKeep);
            currentState = isMediumAM(AMtype) ? st_AMpayload : st_AMreply;
            break;
        }
        case st_AMpayload:{
            gc_payloadSize_t i;
            do{
                #pragma HLS loop_tripcount min=1 max=65536 avg=10
                #pragma HLS PIPELINE
                axis_kernel_in.read(axis_word);
                axis_wordDest = assignWord(axis_word);
                axis_wordDest.dest = AMdst;
                axis_kernel_out.write(axis_wordDest);
            } while(!axis_word.last);
            currentState = st_AMreply;
            break;
        }
        case st_AMreply:{
            if (!isAsyncAM(AMtype)){
                axis_wordNoKeep = createHandlerHeader(AMtype, AMToken, AMsrc,
                    0, H_INCR_MEM, 0);
                axis_wordNoKeep.last = 1;
                axis_handler.write(axis_wordNoKeep);
            }
            currentState = st_AMheader;
            break;
        }
        case st_AMsend:{
            do {
                #pragma HLS loop_tripcount min=1 max=65536 avg=10
                #pragma HLS PIPELINE
                axis_kernel_in.read(axis_word);
                axis_tx.write(axis_word);
            } while(axis_word.last != 1);
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
        CHECK_STATE("st_AMheader", st_AMheader, 0)
        CHECK_STATE("st_sendReplyHeader", st_reply, 1)
        CHECK_STATE("st_AMsend", st_AMsend, 2)
        CHECK_STATE("st_AMpayload", st_AMpayload, 3)
        default: return "Unknown State";
    }
}
#endif
