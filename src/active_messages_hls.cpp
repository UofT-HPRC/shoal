#include <cstddef>
#include "active_messages_hls.hpp"

axis_word_t createHeader(
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

axis_word_t createToken(
    gc_AMToken_t token,
    bool last
){
    axis_word_t axis_word;
    axis_word.data(AM_TOKEN) = token;
    axis_word.last = last;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

axis_word_t createStride(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num,
    gc_AMToken_t token
){
    axis_word_t axis_word;
    axis_word.data(AM_STRIDE_SIZE) = stride;
    axis_word.data(AM_STRIDE_BLK_SIZE) = stride_size;
    axis_word.data(AM_STRIDE_BLK_NUM) = stride_num;
    axis_word.data(AM_TOKEN) = token;
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

axis_word_t createStride(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num
){
    axis_word_t axis_word;
    axis_word.data(AM_STRIDE_SIZE) = stride;
    axis_word.data(AM_STRIDE_BLK_SIZE) = stride_size;
    axis_word.data(AM_STRIDE_BLK_NUM) = stride_num;
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

void sendHandlerArgs(
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

void sendPayloadArgs(
    axis_t & axis_out,
    word_t * payload_args,
    gc_payloadSize_t payloadArgCount
){
    axis_word_t axis_word;
    for (int i = 0; i < payloadArgCount; i++){
        axis_word.data = *(payload_args+i);
        axis_word.last = i == payloadArgCount - 1;
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

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_MEDIUM|AM_FIFO, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
    sendPayloadArgs(axis_out, payload, payloadSize);
}

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_MEDIUM, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    axis_word.data = payloadSize;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
}

void sendlongAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    word_t dst_addr,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_LONG|AM_FIFO, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
    sendPayloadArgs(axis_out, payload, payloadSize);
}

void sendlongAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    word_t dst_addr,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_LONG, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
}

void longStridedAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    gc_stride_t src_stride,
    gc_strideBlockSize_t src_blk_size,
    gc_strideBlockNum_t src_blk_num,
    word_t src_addr,
    gc_stride_t dst_stride,
    gc_strideBlockSize_t dst_blk_size,
    gc_strideBlockNum_t dst_blk_num,
    word_t dst_addr,
    axis_t & axis_out
){
    axis_word_t axis_word;
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_LONG, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createStride(src_stride, src_blk_size, src_blk_num);
    axis_out.write(axis_word);
    axis_word.data = src_addr;
    axis_word.last = 0;
    axis_out.write(axis_word);
    axis_word = createStride(dst_stride, dst_blk_size, dst_blk_num, token);
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = 0;
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
}

bool isShortAM(gc_AMtype_t arg){
    return arg(2,0) == 0b001;
}

bool isMediumAM(gc_AMtype_t arg){
    return arg(2,0) == 0b010;
}

bool isLongxAM(gc_AMtype_t arg){
    return arg(2,2) == 1;
}

bool isLongAM(gc_AMtype_t arg){
    return arg(2,0) == 0b100;
}

bool isLongVectoredAM(gc_AMtype_t arg){
    return arg(2,0) == 0b110;
}

bool isLongStridedAM(gc_AMtype_t arg){
    return arg(2,0) == 0b101;
}

bool isDataFromFIFO(gc_AMtype_t arg){
    return arg[4] == 1;
}

bool isAsyncAM(gc_AMtype_t arg){
    return arg[5] == 1;
}

bool isReplyAM(gc_AMtype_t arg){
    return arg[6] == 1;
}

bool isMediumFIFOAM(gc_AMtype_t arg){
    return isMediumAM(arg) && isDataFromFIFO(arg);
}

bool isLongFIFOAM(gc_AMtype_t arg){
    return isLongAM(arg) && isDataFromFIFO(arg);
}
