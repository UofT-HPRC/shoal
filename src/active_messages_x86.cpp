#include "active_messages.hpp"
#include "am_globals.hpp"

galapagos::stream_packet <word_t> createHeaderBeat(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    gc_AMtype_t type,
    gc_AMargs_t handlerArgCount
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createHeader(src, dst, payloadSize, handlerID, type, handlerArgCount);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

galapagos::stream_packet <word_t> createTokenBeat(
    gc_AMToken_t token,
    bool last
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createToken(token);
    axis_word.last = last;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

galapagos::stream_packet <word_t> createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num,
    gc_AMToken_t token
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createStrided(stride, stride_size, stride_num, token);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

galapagos::stream_packet <word_t> createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createStrided(stride, stride_size, stride_num);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

galapagos::stream_packet <word_t> createVectorBeat(
    gc_srcVectorNum_t srcVectorCount,
    gc_dstVectorNum_t dstVectorCount, 
    gc_vectorSize_t srcSize1,
    gc_vectorSize_t dstSize1,
    gc_AMToken_t token
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = createVectorToken(srcVectorCount, dstVectorCount, srcSize1, dstSize1, token);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}


#if ENABLE_PROFILE == 1
void profile_write(gc_AMsrc_t src, galapagos::stream_packet <word_t> axis_word){
    if (src == KERNEL_PROFILE ){
        std::stringstream ss;
        ss << "PROFILE:W:" << axis_word.data << "\n";
        if (axis_word.last){
            ss << "PROFILE:W:END\n";
        }
        std::cout << ss.str();
    }
}

void profile_read(galapagos::stream_packet <word_t> axis_word){
    std::stringstream ss;
    ss << "PROFILE:R:" << axis_word.data << "\n";
    if (axis_word.last){
        ss << "PROFILE:R:END\n";
    }
    std::cout << ss.str();
}
#endif

#ifdef DEBUG
void printWord(const std::string& prefix, galapagos::stream_packet <word_t> axis_word){
    std::stringstream ss;
    ss << prefix <<
    "Data: " << COLOR(Color::FG_GREEN, hex, axis_word.data) <<
    " Keep: " << COLOR(Color::FG_GREEN, hex, axis_word.keep) <<
    " Last: " << COLOR(Color::FG_GREEN, dec, axis_word.last) <<
    " Dest: " << COLOR(Color::FG_GREEN, dec, axis_word.dest) << "\n";
    std::cout << ss.str();
}
#else
void printWord(const std::string& prefix, galapagos::stream_packet <word_t> axis_word){
}
#endif

inline void _writeWord(
    galapagos::interface <word_t> & axis_out,
    galapagos::stream_packet <word_t> axis_word,
    gc_AMsrc_t src
){
    // printWord("   Sending - ", axis_word);
    axis_word.id = 0;
    axis_out.write(axis_word);
    #if ENABLE_PROFILE == 1
    profile_write(src, axis_word);
    #endif
}

inline void writeWord(
    galapagos::interface <word_t> & axis_out,
    galapagos::stream_packet <word_t> axis_word,
    word_t data,
    bool last,
    gc_AMsrc_t src,
    gc_keep_t keep
){
    axis_word.data = data;
    axis_word.last = last;
    axis_word.keep = keep;
    _writeWord(axis_out, axis_word, src);
}

inline void writeWord(
    galapagos::interface <word_t> & axis_out,
    galapagos::stream_packet <word_t> axis_word,
    gc_AMsrc_t src
){
    _writeWord(axis_out, axis_word, src);
}

inline void writeWord(
    galapagos::interface <word_t> & axis_out,
    galapagos::stream_packet <word_t> axis_word,
    gc_AMsrc_t src,
    gc_AMdst_t dst
){
    axis_word.dest = dst;
    _writeWord(axis_out, axis_word, src);
}

inline void writeWord(
    galapagos::interface <word_t> & axis_out,
    word_t data,
    bool last,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_keep_t keep = GC_DATA_TKEEP
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.data = data;
    axis_word.last = last;
    axis_word.keep = keep;
    axis_word.dest = dst;
    _writeWord(axis_out, axis_word, src);
}

void sendHandlerArgs(
    galapagos::interface <word_t> & axis_out,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    const word_t * handler_args,
    gc_AMargs_t handlerArgCount,
    bool assertLast
){
    galapagos::stream_packet <word_t> axis_word;
    int i;
    for (i = 0; i < handlerArgCount-1; i++){
        writeWord(axis_out, *(handler_args+i), 0, src, dst);
    }
    // i++;
    writeWord(axis_out, *(handler_args+i), assertLast, src, dst);
}

void sendPayloadArgs(
    galapagos::interface <word_t> & axis_out,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    char * payload_args,
    gc_payloadSize_t payloadArgCount,
    bool assertLast
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.dest = dst;
    int i = GC_DATA_BYTES;
    for (i = GC_DATA_BYTES; i < payloadArgCount; i+=GC_DATA_BYTES){
        axis_word.data = *(payload_args+(i-GC_DATA_BYTES));
        axis_word.last = 0;
        axis_word.keep = GC_DATA_TKEEP;
        writeWord(axis_out, axis_word, src);
        // printWord("   Sending - ", axis_word);
        // axis_out.write(axis_word);
    }
    axis_word.data = *(word_t*)((payload_args+(i-GC_DATA_BYTES)));
    axis_word.last = assertLast;
    axis_word.keep = GC_DATA_TKEEP;
    // printWord("   Sending - ", axis_word);
    // axis_out.write(axis_word);
    writeWord(axis_out, axis_word, src);
}

void sendShortAM(
    gc_AMtype_t type,
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    const word_t * handler_args,
    galapagos::interface <word_t> & out
){
    // std::cout << "AM Short message from " << src << " to " << dst << "\n";
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, 0, handlerID, type, handlerArgCount);
    writeWord(out, axis_word, src, dst);
    // axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    // out.write(axis_word);
    axis_word = createTokenBeat(token, handlerArgCount == 0);
    // axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, src, dst, handler_args, handlerArgCount, true);
    }
}

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
){
    // std::cout << "AM Medium message\n";
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    // axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    axis_word = createTokenBeat(token, false);
    // axis_word.dest = dst;
    // printWord("   Sending - ", axis_word);
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, src, dst, handler_args, handlerArgCount, false);
    }
    // sendPayloadArgs(out, src, dst, (char*) payload, payloadSize, true);
}

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
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    axis_word = createTokenBeat(token, false);
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    // axis_word.data = src_addr;
    // axis_word.last = handlerArgCount == 0;
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, src_addr, handlerArgCount == 0, src, dst);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, src, dst, handler_args, handlerArgCount, true);
    }
}

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
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    axis_word = createTokenBeat(token, false);
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    // axis_word.data = dst_addr;
    // axis_word.last = 0;
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, dst_addr, 0, src, dst);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, src, dst, handler_args, handlerArgCount, false);
    }
    // sendPayloadArgs(out, src, dst, (char*) payload, payloadSize, true);
}

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
){
    galapagos::stream_packet <word_t> axis_word;
    // unsigned long long ns_times[5];
    // ns_times[0] = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    // ns_times[1] = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
    // std::cout << "long1: " << (ns_times[1]-ns_times[0])/6.4 << std::endl;
    axis_word = createTokenBeat(token, false);
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    // ns_times[2] = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
    // std::cout << "long2: " << (ns_times[2]-ns_times[1])/6.4 << std::endl;
    // axis_word.data = src_addr;
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, src_addr, 0, src, dst);
    // ns_times[3] = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
    // std::cout << "long3: " << (ns_times[3]-ns_times[2])/6.4 << std::endl;
    // axis_word.data = dst_addr;
    // axis_word.last = handlerArgCount == 0;
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, dst_addr, handlerArgCount == 0, src, dst);
    // ns_times[4] = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
    // std::cout << "long4: " << (ns_times[4]-ns_times[3])/6.4 << std::endl;
    if (handlerArgCount > 0){
        sendHandlerArgs(out, src, dst, handler_args, handlerArgCount, true);
    }
}

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
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    axis_word = createStridedBeat(src_stride, src_blk_size, src_blk_num);
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, axis_word, dst);
    // axis_word.data = src_addr;
    // axis_word.last = 0;
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, src_addr, 0, src, dst);
    axis_word = createStridedBeat(dst_stride, dst_blk_size, dst_blk_num, token);
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, axis_word, src, dst);
    // axis_word.data = dst_addr;
    // axis_word.last = handlerArgCount == 0;
    // axis_word.dest = dst;
    // out.write(axis_word);
    writeWord(out, dst_addr, handlerArgCount == 0, src, dst);
    if (handlerArgCount > 0){
        sendHandlerArgs(out, src, dst, handler_args, handlerArgCount, true);
    }
}

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
){
    galapagos::stream_packet <word_t> axis_word;
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, type, handlerArgCount);
    writeWord(out, axis_word, src, dst);
    
    axis_word = createVectorBeat(srcVectorCount, dstVectorCount, srcSize[0], dstSize[0], token);
    writeWord(out, axis_word, dst);

    writeWord(out, src_addr[0], 0, src, dst);
    writeWord(out, dst_addr[0], (srcVectorCount == 1) && (dstVectorCount == 1) && (handlerArgCount == 0), src, dst);

    int i;
    if (srcVectorCount > 1){
        for(i = 1; i < srcVectorCount-1; i++){
            writeWord(out, srcSize[i], 0, src, dst);
            writeWord(out, src_addr[i], 0, src, dst);
        }
        writeWord(out, srcSize[i], 0, src, dst);
        writeWord(out, src_addr[i], (dstVectorCount == 1) && (handlerArgCount == 0), src, dst);
    }

    if (dstVectorCount > 1){
        for(i = 1; i < dstVectorCount-1; i++){
            writeWord(out, dstSize[i], 0, src, dst);
            writeWord(out, dst_addr[i], 0, src, dst);
        }
        writeWord(out, dstSize[i], 0, src, dst);
        writeWord(out, dst_addr[i], handlerArgCount == 0, src, dst);
    }

    if (handlerArgCount > 0){
        sendHandlerArgs(out, src, dst, handler_args, handlerArgCount, true);
    }
}