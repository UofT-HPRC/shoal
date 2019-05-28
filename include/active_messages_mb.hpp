#if !defined(SHOAL_INCLUDE_ACTIVE_MESSAGES_MB_)
#define SHOAL_INCLUDE_ACTIVE_MESSAGES_MB_

#include "fsl.h"
#include "xil_io.h"

#include "config.hpp"
#include "hls_types.hpp"

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

#endif // SHOAL_INCLUDE_ACTIVE_MESSAGES_MB_
