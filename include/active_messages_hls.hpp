#ifndef SHOAL_INCLUDE_ACTIVE_MESSAGES_HLS_H_
#define SHOAL_INCLUDE_ACTIVE_MESSAGES_HLS_H_

#include "config.hpp"
#include "hls_types.hpp"

#if defined(__x86_64__)

/*******************************************************************************
 * Active Message constructors
*******************************************************************************/

axis_word_t createHeader(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    int type,
    gc_AMargs_t handlerArgCount
);

axis_word_t createToken(
    gc_AMToken_t token,
    bool last
);

axis_word_t createStride(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num,
    gc_AMToken_t token
);

axis_word_t createStride(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num
);

void sendHandlerArgs(
    axis_t & axis_out,
    word_t * handler_args,
    gc_AMargs_t handlerArgCount
);

void sendPayloadArgs(
    axis_t & axis_out,
    word_t * payload_args,
    gc_payloadSize_t payloadArgCount
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

void sendlongStridedAM(
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


/*******************************************************************************
 * Active Message type check
*******************************************************************************/

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

#endif

#endif // SHOAL_INCLUDE_ACTIVE_MESSAGES_HLS_H_
