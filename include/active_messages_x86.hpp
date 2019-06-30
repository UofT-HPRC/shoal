#if !defined(SHOAL_INCLUDE_ACTIVE_MESSAGES_X86_)
#define SHOAL_INCLUDE_ACTIVE_MESSAGES_X86_

#include "config.hpp"
#include "hls_types.hpp"

#define CPU
#include "galapagos_stream.hpp"

#include "user_config.hpp"

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

void printWord(const std::string& prefix, galapagos::stream_packet <word_t> axis_word);

void sendHandlerArgs(
    galapagos::stream <word_t> & axis_out,
    gc_AMdst_t dst,
    word_t * handler_args,
    gc_AMargs_t handlerArgCount,
    bool assertLast
);

void sendPayloadArgs(
    galapagos::stream <word_t> & axis_out,
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
    word_t * handler_args,
    galapagos::stream <word_t> & out
);

void sendMediumAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    galapagos::stream <word_t> & out
);

void sendMediumAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    galapagos::stream <word_t> & out
);

void sendLongAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    word_t dst_addr,
    galapagos::stream <word_t> & out
);

void sendLongAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    word_t dst_addr,
    galapagos::stream <word_t> & out
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
    galapagos::stream <word_t> & out
);

#if ENABLE_PROFILE == 1
void profile_read(galapagos::stream_packet <word_t> axis_word);
#endif

#endif // SHOAL_INCLUDE_ACTIVE_MESSAGES_X86_
