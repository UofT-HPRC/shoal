#include <cstddef>
#include "active_messages.hpp"

word_t createHeader(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    gc_AMtype_t type,
    gc_AMargs_t handlerArgCount
){
    word_t header = ((word_t)src << AM_SRC_LOWER) | 
        ((word_t)dst << AM_DST_LOWER) |
        ((word_t)payloadSize << AM_PAYLOAD_SIZE_LOWER) | 
        ((word_t)handlerID << AM_HANDLER_LOWER) |
        ((word_t)type << AM_TYPE_LOWER) | 
        ((word_t)handlerArgCount << AM_HANDLER_ARGS_LOWER);
    return header;
}

word_t createToken(
    gc_AMToken_t token
){
    word_t word = (word_t)token << AM_TOKEN_LOWER;
    return word;
}

word_t createVectorToken(
    gc_srcVectorNum_t srcVectorCount,
    gc_dstVectorNum_t dstVectorCount,
    gc_vectorSize_t srcSize1,
    gc_vectorSize_t dstSize1,
    gc_AMToken_t token
){
    word_t word = ((word_t)srcVectorCount << AM_SRC_VECTOR_NUM_LOWER) |
        ((word_t)dstVectorCount << AM_DST_VECTOR_NUM_LOWER) |
        ((word_t)srcSize1 << AM_SRC_VECTOR_SIZE_HEAD_LOWER) |
        ((word_t)dstSize1 << AM_DST_VECTOR_SIZE_HEAD_LOWER) |
        ((word_t)token << AM_TOKEN_LOWER);
    return word;
}

word_t createStrided(
    gc_stride_t stride,
    gc_strideBlockSize_t blockSize,
    gc_strideBlockNum_t blockNum,
    gc_AMToken_t token
){
    word_t word = ((word_t)stride << AM_STRIDE_SIZE_LOWER) |
        ((word_t)blockSize << AM_STRIDE_BLK_SIZE_LOWER) |
        ((word_t)blockNum << AM_STRIDE_BLK_NUM_LOWER) |
        ((word_t)token << AM_TOKEN_LOWER);
    return word;
}

word_t createStrided(
    gc_stride_t stride,
    gc_strideBlockSize_t blockSize,
    gc_strideBlockNum_t blockNum
){
    word_t word = ((word_t)stride << AM_STRIDE_SIZE_LOWER) |
        ((word_t)blockSize << AM_STRIDE_BLK_SIZE_LOWER) |
        ((word_t)blockNum << AM_STRIDE_BLK_NUM_LOWER);
    return word;
}

word_t createStridedSrcBody(
    gc_vectorSize_t size
){
    return ((word_t)size << AM_SRC_VECTOR_SIZE_BODY_LOWER);
}

word_t createStridedDstBody(
    gc_vectorSize_t size
){
    return ((word_t)size << AM_DST_VECTOR_SIZE_BODY_LOWER);
}

bool isShortAM(gc_AMtype_t arg){
    return (arg & 0x7) == 1;
}

bool isMediumAM(gc_AMtype_t arg){
    return (arg & 0x7) == 2;
}

bool isLongxAM(gc_AMtype_t arg){
    return (arg & 0x4) == 0x4;
}

bool isLongAM(gc_AMtype_t arg){
    return (arg & 0x7) == 4;
}

bool isLongVectoredAM(gc_AMtype_t arg){
    return (arg & 0x7) == 6;
}

bool isLongStridedAM(gc_AMtype_t arg){
    return (arg & 0x7) == 5;
}

bool isDataFromFIFO(gc_AMtype_t arg){
    return (arg & 0x10) == 0x10;
}

bool isAsyncAM(gc_AMtype_t arg){
    return (arg & 0x20) == 0x20;
}

bool isReplyAM(gc_AMtype_t arg){
    return (arg & 0x40) == 0x40;
}

bool isMediumFIFOAM(gc_AMtype_t arg){
    return isMediumAM(arg) && isDataFromFIFO(arg);
}

bool isLongFIFOAM(gc_AMtype_t arg){
    return isLongAM(arg) && isDataFromFIFO(arg);
}

#if defined(__HLS__)

#include "active_messages_hls.cpp"

#elif defined(__MICROBLAZE__) // x86_64

#include "active_messages_mb.cpp"

#elif defined(__x86_64__)

#include "active_messages_x86.cpp"

#endif // architecture (MB / x86_64 / HLS)
