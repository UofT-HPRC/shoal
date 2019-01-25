#if !defined(ACTIVE_MESSAGES_H)
#define ACTIVE_MESSAGES_H

#include "config.hpp"

inline long long createHeader(
    int src,
    int dst,
    int payloadSize,
    int handlerID,
    int type,
    int handlerArgCount
){
    long long header = ((long long)src << AM_SRC_LOWER) + 
        ((long long)dst << AM_DST_LOWER) + 
        ((long long)payloadSize << AM_PAYLOAD_SIZE_LOWER) + 
        ((long long)handlerID << AM_HANDLER_LOWER) +
        ((long long)type << AM_TYPE_LOWER) + 
        ((long long)handlerArgCount << AM_HANDLER_ARGS_LOWER);
    // long long header = (src << AM_SRC_LOWER);
    // header += (dst << AM_DST_LOWER);
    // header += (payloadSize << AM_PAYLOAD_SIZE_LOWER);
    // header += (handlerID << AM_HANDLER_LOWER);
    // header += (type << AM_TYPE_LOWER);
    // header += (handlerArgCount << AM_HANDLER_ARGS_LOWER);
    return header;
}

inline long long createToken(
    int token
){
    long long word = (long long)token << AM_TOKEN_LOWER;
    return word;
}

// #define __HLS__

#if defined(__HLS__)

#include "../GAScore/include/utilities.hpp"

void writeWord(
    word_t data,
    bool last,
    bool keep,

){

}


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

inline axis_word_t createStride(
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

inline axis_word_t createStride(
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

inline void sendPayloadArgs(
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

void sendlong longAM(
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
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_long long|AM_FIFO, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
    sendPayloadArgs(axis_out, payload, payloadSize);
}

void sendlong longAM(
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
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_long long, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createToken(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount);
}

void sendlong longStridedAM(
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
    axis_word = createHeader(src, dst, payloadSize, handlerID, AM_long long, handlerArgCount);
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

#elif defined(__MICROBLAZE__)

#include "fsl.h"
// #include <cstdint>

bool checkKernelEmpty(){
    long word_0;
    int return_val;
    getfslx(word_0, KERNEL_LINK, FSL_NONBLOCKING);
    fsl_isinvalid(return_val);
    return return_val;
}

#if defined(ENABLE_NETWORK)

bool checkNetEmpty(){
    long word_0;
    int return_val;
    getfslx(word_0, NET_LINK, FSL_NONBLOCKING);
    fsl_isinvalid(return_val);
    return return_val;
}

long long readNetwork(){
    long word_0, word_1;
    getfslx(word_0, NET_LINK, FSL_DEFAULT);
    getfslx(word_1, NET_LINK, FSL_DEFAULT);
    long long word = ((long long)word_1 << 32);
    word |= (long long)word_0;
    return word;
}

void writeNetwork (long long word, bool assertLast){
    int word_0 = word & 0xFFFFFFFF;
    int word_1 = (word >> 32) & 0xFFFFFFFF;
    putfslx(word_0, NET_LINK, FSL_DEFAULT);
    if (assertLast){
        putfslx(word_1, NET_LINK, FSL_CONTROL);
    } else {
        putfslx(word_1, NET_LINK, FSL_DEFAULT);
    }
}

#endif

long long readKernel(){
    long word_0, word_1;
    getfslx(word_0, KERNEL_LINK, FSL_DEFAULT);
    getfslx(word_1, KERNEL_LINK, FSL_DEFAULT);
    long long word = ((long long)word_1 << 32);
    word |= (long long)word_0;
    return word;
}

void writeKernel(long long word, bool assertLast){
    int word_0 = word & 0xFFFFFFFF;
    int word_1 = (word >> 32) & 0xFFFFFFFF;
    putfslx(word_0, KERNEL_LINK, FSL_DEFAULT);
    if (assertLast){
        putfslx(word_1, KERNEL_LINK, FSL_CONTROL);
    } else {
        putfslx(word_1, KERNEL_LINK, FSL_DEFAULT);
    }
}

void sendArgs(
    void * args,
    int argCount,
    bool assertLast
){
    long long word;
    int header_0;
    int header_1;
    int i;
    for (i = 0; i < argCount-1; i++){
        word = *((long long *)(args)+i);
        header_0 = word & 0xFFFFFFFF;
        header_1 = (word >> 32) & 0xFFFFFFFF;
        putfslx(header_0, KERNEL_LINK, FSL_DEFAULT);
        putfslx(header_1, KERNEL_LINK, FSL_DEFAULT);
    }
    word = *((long long *)(args)+i);
    header_0 = word & 0xFFFFFFFF;
    header_1 = (word >> 32) & 0xFFFFFFFF;
    putfslx(header_0, KERNEL_LINK, FSL_DEFAULT);
    if (assertLast){
        putfslx(header_1, KERNEL_LINK, FSL_CONTROL);
    } else {
        putfslx(header_1, KERNEL_LINK, FSL_DEFAULT);
    }
}

void writeReg(int address, int offset, int value){
    Xil_Out32(address + offset, value);
}

int readReg(int address, int offset){
    return Xil_In32(address + offset);
}

void sendShortAM(
    int src,
    int dst,
    int token,
    int handlerID,
    int handlerArgCount,
    void * handler_args
){
    long long header = createHeader(src, dst, 0, handlerID, AM_SHORT, handlerArgCount);
    writeKernel(header, false);
    header = createToken(token);
    if (handlerArgCount  > 0){
        writeKernel(header, false);
        sendArgs(handler_args, handlerArgCount, true);
    } else {
        writeKernel(header, true);
    }
}

void sendMediumAM(
    int src,
    int dst,
    int token,
    int handlerID,
    int handlerArgCount,
    void * handler_args,
    int payloadSize,
    void * payload
){
    long long header = createHeader(src, dst, payloadSize, handlerID, AM_MEDIUM|AM_FIFO, handlerArgCount);
    writeKernel(header, false);
    header = createToken(token);
    writeKernel(header, false);
    if (handlerArgCount  > 0){
        sendArgs(handler_args, handlerArgCount, false);
    }
    sendArgs(payload, payloadSize, true);
}

void sendMediumAM(
    int src,
    int dst,
    int token,
    int handlerID,
    int handlerArgCount,
    void * handler_args,
    int payloadSize,
    long long src_addr
){
    long long header = createHeader(src, dst, payloadSize, handlerID, AM_MEDIUM, handlerArgCount);
    writeKernel(header, false);
    header = createToken(token);
    writeKernel(header, false);
    header = src_addr;
    if (handlerArgCount > 0){
        writeKernel(header, false);
        sendArgs(handler_args, handlerArgCount, true);
    } else {
        writeKernel(header, true);
    }
}

void sendLongAM(
    int src,
    int dst,
    int token,
    int handlerID,
    int handlerArgCount,
    void * handler_args,
    int payloadSize,
    void * payload,
    long long dst_addr
){
    long long word = createHeader(src, dst, payloadSize, handlerID, AM_LONG|AM_FIFO, handlerArgCount);
    writeKernel(word, false);
    word = createToken(token);
    writeKernel(word, false);
    writeKernel(dst_addr, false);
    if (handlerArgCount > 0){
        sendArgs(handler_args, handlerArgCount, false);
    }
    sendArgs(payload, payloadSize, true);
}

void sendLongAM(
    int src,
    int dst,
    int token,
    int handlerID,
    int handlerArgCount,
    void * handler_args,
    int payloadSize,
    long long src_addr,
    long long dst_addr
){
    long long word = createHeader(src, dst, payloadSize, handlerID, AM_LONG, handlerArgCount);
    writeKernel(word, false);
    word = createToken(token);
    writeKernel(word, false);
    writeKernel(src_addr, false);
    if (handlerArgCount > 0){
        writeKernel(dst_addr, false);
        sendArgs(handler_args, handlerArgCount, true);
    } else {
        writeKernel(dst_addr, true);
    }
}

#endif // architecture

#endif // ACTIVE_MESSAGES_H
