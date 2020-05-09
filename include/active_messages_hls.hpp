#pragma once

#include "config.hpp"
#include "hls_types.hpp"

#ifndef __HLS__
#define __HLS__
#endif
#ifdef CPU
#undef CPU
#endif
#include "galapagos_packet.h"

galapagos::stream_packet <word_t> createHeaderBeat(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    gc_AMtype_t type,
    gc_AMargs_t handlerArgCount
);

galapagos::stream_packet <word_t> createTokenBeat(
    gc_AMToken_t token,
    bool last
);

galapagos::stream_packet <word_t> createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num,
    gc_AMToken_t token
);

galapagos::stream_packet <word_t> createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num
);

void writeWord(
    galapagos::interface <word_t> & axis_out,
    galapagos::stream_packet <word_t> axis_word,
    gc_AMdst_t dst
);

void writeWord(
    galapagos::interface <word_t> & axis_out,
    word_t data,
    bool last,
    gc_AMdst_t dst
);

void writeWord(
    galapagos::interface <word_t> & axis_out,
    word_t data,
    bool last,
    gc_AMdst_t dst,
    gc_keep_t keep
);

void sendHandlerArgs(
    galapagos::interface <word_t> & axis_out,
    gc_AMdst_t dst,
    const word_t * handler_args,
    gc_AMargs_t handlerArgCount,
    bool assertLast
);

void sendPayloadArgs(
    galapagos::interface <word_t> & axis_out,
    gc_AMdst_t dst,
    char * payload_args,
    gc_payloadSize_t payloadArgCount,
    bool assertLast
);

void sendShortAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    galapagos::interface <word_t> & out
);

void sendMediumAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    // const word_t * payload,
    galapagos::interface <word_t> & out
);

void sendMediumAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    galapagos::interface <word_t> & out
);

void sendLongAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    // const word_t * payload,
    word_t dst_addr,
    galapagos::interface <word_t> & out
);

void sendLongAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    word_t dst_addr,
    galapagos::interface <word_t> & out
);

void longStridedAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    gc_stride_t src_stride,
    gc_strideBlockSize_t src_blk_size,
    gc_strideBlockNum_t src_blk_num,
    word_t src_addr,
    gc_stride_t dst_stride,
    gc_strideBlockSize_t dst_blk_size,
    gc_strideBlockNum_t dst_blk_num,
    word_t dst_addr,
    galapagos::interface <word_t> & out
);

void longVectorAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    gc_payloadSize_t payloadSize,
    gc_srcVectorNum_t srcVectorCount,
    gc_dstVectorNum_t dstVectorCount,
    const gc_vectorSize_t * srcSize,
    const gc_vectorSize_t * dstSize,
    const word_t * src_addr,
    const word_t * dst_addr,
    galapagos::interface <word_t> & out
);