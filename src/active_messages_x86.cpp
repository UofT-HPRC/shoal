#include "active_messages.hpp"

galapagos::stream_packet <word_t> createHeaderBeat(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    gc_AMtype_t type,
    gc_AMargs_t handlerArgCount
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createHeader(src, dst, payloadSize, handlerID, type, handlerArgCount);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

galapagos::stream_packet <word_t> createTokenBeat(
    gc_AMToken_t token,
    bool last
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createToken(token);
    axis_word.last = last;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

void sendHandlerArgs(
    galapagos::stream <word_t> & axis_out,
    gc_AMdst_t dst,
    word_t * handler_args,
    gc_AMargs_t handlerArgCount,
    bool assertLast
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.dest = dst;
    int i;
    for (i = 0; i < handlerArgCount-1; i++){
        axis_word.data = *(handler_args+i);
        axis_word.last = 0;
        axis_word.keep = GC_DATA_TKEEP;
        axis_out.write(axis_word);;
    }
    i++;
    axis_word.data = *(handler_args+i);
    axis_word.last = assertLast;
    axis_word.keep = GC_DATA_TKEEP;
    axis_out.write(axis_word);;
}

void sendPayloadArgs(
    galapagos::stream <word_t> & axis_out,
    gc_AMdst_t dst,
    char * payload_args,
    gc_payloadSize_t payloadArgCount,
    bool assertLast
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.dest = dst;
    int i = GC_DATA_BYTES;
    for (i = GC_DATA_BYTES; i < payloadArgCount; i+=GC_DATA_BYTES){
        axis_word.data = *(payload_args+(i-GC_DATA_BYTES));
        axis_word.last = 0;
        axis_word.keep = GC_DATA_TKEEP;
        axis_out.write(axis_word);;
    }
    axis_word.data = *(word_t*)((payload_args+(i-GC_DATA_BYTES)));
    axis_word.last = assertLast;
    axis_word.keep = GC_DATA_TKEEP;
    axis_out.write(axis_word);
    std::cout << "Sending " << std::hex << axis_word.data << "\n";
}

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    galapagos::stream <word_t> & out
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize + GC_DATA_BYTES, handlerID, AM_MEDIUM|AM_FIFO, handlerArgCount);
    axis_word.dest = dst;
    std::cout << "Sending " << std::hex << axis_word.data << "\n";
    out.write(axis_word);
    axis_word = createTokenBeat(token, false);
    axis_word.dest = dst;
    out.write(axis_word);
    std::cout << "Sending " << std::hex << axis_word.data << "\n";
    if (handlerArgCount > 0){
        sendHandlerArgs(out, dst, handler_args, handlerArgCount, false);
    }
    sendPayloadArgs(out, dst, (char*) payload, payloadSize, true);
}

void **handlertable = nullptr; // provided by user application, handler functions to run
mutex_t mutex_nodeInit; // mutex to initialize the node (per thread)

thread_local gasnet_nodedata_t* nodedata = nullptr; // used to hold a thread-local copy of gasnet_nodedata_all
thread_local void** gasnet_shared_mem = nullptr;

std::atomic_bool** kernel_done = nullptr;
