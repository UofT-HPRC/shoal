#include "active_messages.hpp"

inline axis_word_t createHeader(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    int type,
    gc_AMargs_t handlerArgCount
){
    axis_word_t axis_word;
    axis_word.data(AM_SRC) = src;
    axis_word.data(AM_DST) = dst;
    axis_word.data(AM_PAYLOAD_SIZE) = payloadSize;
    axis_word.data(AM_HANDLER) = handlerID;
    axis_word.data(AM_TYPE) = type;
    axis_word.data(AM_HANDLER_ARGS) = handlerArgCount;
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

inline axis_word_t createToken(
    gc_AMToken_t token,
    bool last
){
    axis_word_t axis_word;
    axis_word.data(AM_TOKEN) = token;
    axis_word.last = last;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

inline void sendHandlerArgs(
    axis_t & axis_out,
    word_t * handler_args,
    gc_AMargs_t handlerArgCount
){
    axis_word_t axis_word;
    for (int i = 0; i < handlerArgCount; i++){
        axis_word.data = *(handler_args+i);
        axis_word.last = i == handlerArgCount - 1;
        axis_word.keep = GC_DATA_TKEEP;
        axis_out.write(axis_word);;
    }
}

void sendShortAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, 0, handlerID, AM_SHORT, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
}