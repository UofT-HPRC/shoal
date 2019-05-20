#ifndef SHOAL_INCLUDE_ACTIVE_MESSAGES_H_
#define SHOAL_INCLUDE_ACTIVE_MESSAGES_H_

#include "config.hpp"
#include "hls_types.hpp"

word_t createHeader(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    gc_AMtype_t type,
    gc_AMargs_t handlerArgCount
);

word_t createToken(gc_AMToken_t token);

word_t createVectorToken(
    gc_srcVectorNum_t srcVectorCount,
    gc_dstVectorNum_t dstVectorCount,
    gc_vectorSize_t srcSize1,
    gc_vectorSize_t dstSize1,
    gc_AMToken_t token
);

word_t createStrided(
    gc_stride_t stride,
    gc_strideBlockSize_t blockSize,
    gc_strideBlockNum_t blockNum,
    gc_AMToken_t token
);

word_t createStrided(
    gc_stride_t stride,
    gc_strideBlockSize_t blockSize,
    gc_strideBlockNum_t blockNum
);

word_t createStridedSrcBody(gc_vectorSize_t size);

word_t createStridedDstBody(gc_vectorSize_t size);

bool isShortAM(gc_AMtype_t arg);

bool isMediumAM(gc_AMtype_t arg);

bool isLongxAM(gc_AMtype_t arg);

bool isLongAM(gc_AMtype_t arg);

bool isLongVectoredAM(gc_AMtype_t arg);

bool isLongStridedAM(gc_AMtype_t arg);

bool isDataFromFIFO(gc_AMtype_t arg);

bool isAsyncAM(gc_AMtype_t arg);

bool isReplyAM(gc_AMtype_t arg);

bool isMediumFIFOAM(gc_AMtype_t arg);

bool isLongFIFOAM(gc_AMtype_t arg);

#if defined(__MICROBLAZE__)

#include "fsl.h"
#include "xil_io.h"

bool checkKernelEmpty();

#if defined(ENABLE_NETWORK)

bool checkNetEmpty();

word_t readNetwork();

void writeNetwork (word_t word, bool assertLast);

#endif

word_t readKernel();

void writeKernel(word_t word, bool assertLast);

void sendArgs(
    void * args,
    int argCount,
    bool assertLast
);

void writeReg(int address, int offset, int value);

int readReg(int address, int offset);

void sendShortAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args
);

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload
);

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr
);

void sendLongAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    word_t dst_addr
);

void sendLongAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    word_t dst_addr
);

void sendVectorAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    gc_srcVectorNum_t srcVectorCount,
    gc_dstVectorNum_t dstVectorCount,
    gc_vectorSize_t * srcSize,
    gc_vectorSize_t * dstSize,
    word_t * src_addr,
    word_t * dst_addr
);

void sendStridedAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    gc_stride_t srcStride,
    gc_strideBlockSize_t srcBlockSize,
    gc_strideBlockNum_t srcBlockNum,
    word_t src_addr,
    gc_stride_t dstStride,
    gc_strideBlockSize_t dstBlockSize,
    gc_strideBlockNum_t dstBlockNum,
    word_t dst_addr
);

#elif defined(__HLS__)

axis_word_t createHeaderBeat(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    gc_AMtype_t type,
    gc_AMargs_t handlerArgCount
);

axis_word_t createTokenBeat(
    gc_AMToken_t token,
    bool last
);

axis_word_t createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num,
    gc_AMToken_t token
);

axis_word_t createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num
);

void sendHandlerArgs(
    axis_t & axis_out,
    word_t * handler_args,
    gc_AMargs_t handlerArgCount,
    bool assertLast
);

void sendPayloadArgs(
    axis_t & axis_out,
    word_t * payload_args,
    gc_payloadSize_t payloadArgCount,
    bool assertLast
);

void sendShortAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    axis_t & axis_out
);

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
);

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
);

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
);

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
);

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
);

#elif defined(__x86_64__)

#endif // architecture

#endif // SHOAL_INCLUDE_ACTIVE_MESSAGES_H_
