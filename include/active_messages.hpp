#ifndef SHOAL_INCLUDE_ACTIVE_MESSAGES_H_
#define SHOAL_INCLUDE_ACTIVE_MESSAGES_H_

#include "config.hpp"
#include "hls_types.hpp"

// #define DEBUG

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

#if defined(__HLS__)

#include "active_messages_hls.hpp"

#elif defined(__MICROBLAZE__)

#include "active_messages_mb.hpp"

#elif defined(__x86_64__)

#include "active_messages_x86.hpp"

#endif // architecture

#endif // SHOAL_INCLUDE_ACTIVE_MESSAGES_H_
