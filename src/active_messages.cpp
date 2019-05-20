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
    word_t header = ((word_t)src << AM_SRC_LOWER) + 
        ((word_t)dst << AM_DST_LOWER) + 
        ((word_t)payloadSize << AM_PAYLOAD_SIZE_LOWER) + 
        ((word_t)handlerID << AM_HANDLER_LOWER) +
        ((word_t)type << AM_TYPE_LOWER) + 
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
    word_t word = ((word_t)srcVectorCount << AM_SRC_VECTOR_NUM_LOWER) +
        ((word_t)dstVectorCount << AM_DST_VECTOR_NUM_LOWER) +
        ((word_t)srcSize1 << AM_SRC_VECTOR_SIZE_HEAD_LOWER) +
        ((word_t)dstSize1 << AM_DST_VECTOR_SIZE_HEAD_LOWER) +
        ((word_t)token << AM_TOKEN_LOWER);
    return word;
}

word_t createStrided(
    gc_stride_t stride,
    gc_strideBlockSize_t blockSize,
    gc_strideBlockNum_t blockNum,
    gc_AMToken_t token
){
    word_t word = ((word_t)stride << AM_STRIDE_SIZE_LOWER) +
        ((word_t)blockSize << AM_STRIDE_BLK_SIZE_LOWER) +
        ((word_t)blockNum << AM_STRIDE_BLK_NUM_LOWER) +
        ((word_t)token << AM_TOKEN_LOWER);
    return word;
}

word_t createStrided(
    gc_stride_t stride,
    gc_strideBlockSize_t blockSize,
    gc_strideBlockNum_t blockNum
){
    word_t word = ((word_t)stride << AM_STRIDE_SIZE_LOWER) +
        ((word_t)blockSize << AM_STRIDE_BLK_SIZE_LOWER) +
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
    return (arg & 0x4) == 1;
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

#if defined(__x86_64__) || defined(__MICROBLAZE__)

#if defined(__MICROBLAZE__)

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

word_t readNetwork(){
    long word_0, word_1;
    getfslx(word_0, NET_LINK, FSL_DEFAULT);
    getfslx(word_1, NET_LINK, FSL_DEFAULT);
    word_t word = ((word_t)word_1 << 32);
    word |= (word_t)word_0;
    return word;
}

void writeNetwork (word_t word, bool assertLast){
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

word_t readKernel(){
    long word_0, word_1;
    getfslx(word_0, KERNEL_LINK, FSL_DEFAULT);
    getfslx(word_1, KERNEL_LINK, FSL_DEFAULT);
    word_t word = ((word_t)word_1 << 32);
    word |= (word_t)word_0;
    return word;
}

void writeKernel(word_t word, bool assertLast){
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
    word_t word;
    int header_0;
    int header_1;
    int i;
    for (i = 0; i < argCount-1; i++){
        word = *((word_t *)(args)+i);
        header_0 = word & 0xFFFFFFFF;
        header_1 = (word >> 32) & 0xFFFFFFFF;
        putfslx(header_0, KERNEL_LINK, FSL_DEFAULT);
        putfslx(header_1, KERNEL_LINK, FSL_DEFAULT);
    }
    word = *((word_t *)(args)+i);
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
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args
){
    word_t header = createHeader(src, dst, 0, handlerID, AM_SHORT, handlerArgCount);
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
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload
){
    word_t header = createHeader(src, dst, payloadSize, handlerID, AM_MEDIUM|AM_FIFO, handlerArgCount);
    writeKernel(header, false);
    header = createToken(token);
    writeKernel(header, false);
    if (handlerArgCount  > 0){
        sendArgs(handler_args, handlerArgCount, false);
    }
    sendArgs(payload, payloadSize, true);
}

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr
){
    word_t header = createHeader(src, dst, payloadSize, handlerID, AM_MEDIUM, handlerArgCount);
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
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    word_t dst_addr
){
    word_t word = createHeader(src, dst, payloadSize, handlerID, AM_LONG|AM_FIFO, handlerArgCount);
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
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t src_addr,
    word_t dst_addr
){
    word_t word = createHeader(src, dst, payloadSize, handlerID, AM_LONG, handlerArgCount);
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
){
    word_t word = createHeader(src, dst, payloadSize, handlerID, AM_VECTOR, handlerArgCount);
    writeKernel(word, false);
    word = createVectorToken(srcVectorCount, dstVectorCount, *srcSize, *dstSize, token);
    writeKernel(word, false);
    writeKernel(*src_addr, false);
    writeKernel(*dst_addr, false);

    if (srcVectorCount > 1){
        for(int i = 1; i < srcVectorCount - 1; i++){
            word = createStridedSrcBody(*(++srcSize));
            writeKernel(word, false);
            writeKernel(*(++src_addr), false);
        }
        word = createStridedSrcBody(*(++srcSize));
        writeKernel(word, false);
        if (dstVectorCount == 1 && handlerArgCount == 0){
            writeKernel(*(++src_addr), true);
        } else {
            writeKernel(*(++src_addr), false);
        }
    }

    if (dstVectorCount > 1){
        for(int i = 1; i < dstVectorCount - 1; i++){
            word = createStridedDstBody(*(++dstSize));
            writeKernel(word, false);
            writeKernel(*(++dst_addr), false);
        }
        word = createStridedSrcBody(*(++dstSize));
        writeKernel(word, false);
        if (handlerArgCount == 0){
            writeKernel(*(++dst_addr), true);
        } else {
            writeKernel(*(++dst_addr), false);
        }
    }

    if (handlerArgCount > 0){
        sendArgs(handler_args, handlerArgCount, true);
    }
}

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
){
    word_t word = createHeader(src, dst, payloadSize, handlerID, AM_STRIDE, handlerArgCount);
    writeKernel(word, false);
    word = createStrided(srcStride, srcBlockSize, srcBlockNum);
    writeKernel(word, false);
    writeKernel(src_addr, false);
    word = createStrided(dstStride, dstBlockSize, dstBlockNum);
    writeKernel(word, false);
    if (handlerArgCount > 0){
        writeKernel(dst_addr, false);
        sendArgs(handler_args, handlerArgCount, true);
    } else {
        writeKernel(dst_addr, true);
    }
}

#elif defined(__HLS__)

axis_word_t createHeaderBeat(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    gc_AMtype_t type,
    gc_AMargs_t handlerArgCount
){
    axis_word_t axis_word;
    axis_word.data = createHeader(src, dst, payloadSize, handlerID, type, handlerArgCount);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

axis_word_t createTokenBeat(
    gc_AMToken_t token,
    bool last
){
    axis_word_t axis_word;
    axis_word.data = createToken(token);
    axis_word.last = last;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

axis_word_t createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num,
    gc_AMToken_t token
){
    axis_word_t axis_word;
    axis_word.data = createStrided(stride, stride_size, stride_num, token);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

axis_word_t createStridedBeat(
    gc_stride_t stride,
    gc_strideBlockSize_t stride_size,
    gc_strideBlockNum_t stride_num
){
    axis_word_t axis_word;
    axis_word.data = createStrided(stride, stride_size, stride_num);
    axis_word.last = 0;
    axis_word.keep = GC_DATA_TKEEP;
    return axis_word;
}

void sendHandlerArgs(
    axis_t & axis_out,
    word_t * handler_args,
    gc_AMargs_t handlerArgCount,
    bool assertLast
){
    axis_word_t axis_word;
    int i;
    for (i = 0; i < handlerArgCount-1; i++){
        axis_word.data = *(handler_args+i);
        axis_word.last = 0;
        axis_word.keep = GC_DATA_TKEEP;
        axis_out.write(axis_word);;
    }
    i++;
    axis_word.data = *(handler_args+i);
    axis_word.last = assertLast;
    axis_word.keep = GC_DATA_TKEEP;
    axis_out.write(axis_word);;
}

void sendPayloadArgs(
    axis_t & axis_out,
    word_t * payload_args,
    gc_payloadSize_t payloadArgCount,
    bool assertLast
){
    axis_word_t axis_word;
    int i;
    for (i = 0; i < payloadArgCount-1; i++){
        axis_word.data = *(payload_args+i);
        axis_word.last = 0;
        axis_word.keep = GC_DATA_TKEEP;
        axis_out.write(axis_word);;
    }
    i++;
    axis_word.data = *(payload_args+i);
    axis_word.last = assertLast;
    axis_word.keep = GC_DATA_TKEEP;
    axis_out.write(axis_word);;
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
    axis_word = createHeaderBeat(src, dst, 0, handlerID, AM_SHORT, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createTokenBeat(token, handlerArgCount == 0);
    axis_out.write(axis_word);
    sendHandlerArgs(axis_out, handler_args, handlerArgCount, true);
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
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, AM_MEDIUM|AM_FIFO, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createTokenBeat(token, false);
    axis_out.write(axis_word);
    if (handlerArgCount > 0){
        sendHandlerArgs(axis_out, handler_args, handlerArgCount, false);
    }
    sendPayloadArgs(axis_out, payload, payloadSize, true);
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
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, AM_MEDIUM, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createTokenBeat(token, false);
    axis_out.write(axis_word);
    axis_word.data = src_addr;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    if (handlerArgCount > 0){
        sendHandlerArgs(axis_out, handler_args, handlerArgCount, true);
    }
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
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, AM_LONG|AM_FIFO, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createTokenBeat(token, false);
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = 0;
    axis_out.write(axis_word);
    if (handlerArgCount > 0){
        sendHandlerArgs(axis_out, handler_args, handlerArgCount, false);
    }
    sendPayloadArgs(axis_out, payload, payloadSize, true);
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
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, AM_LONG, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createTokenBeat(token, false);
    axis_out.write(axis_word);
    axis_word.data = src_addr;
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    if (handlerArgCount > 0){
        sendHandlerArgs(axis_out, handler_args, handlerArgCount, true);
    }
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
    axis_word = createHeaderBeat(src, dst, payloadSize, handlerID, AM_LONG, handlerArgCount);
    axis_out.write(axis_word);
    axis_word = createStridedBeat(src_stride, src_blk_size, src_blk_num);
    axis_out.write(axis_word);
    axis_word.data = src_addr;
    axis_word.last = 0;
    axis_out.write(axis_word);
    axis_word = createStridedBeat(dst_stride, dst_blk_size, dst_blk_num, token);
    axis_out.write(axis_word);
    axis_word.data = dst_addr;
    axis_word.last = handlerArgCount == 0;
    axis_out.write(axis_word);
    if (handlerArgCount == 0){
        sendHandlerArgs(axis_out, handler_args, handlerArgCount, true);
    }
}

#elif defined(__x86_64__)

#endif // architecture

#endif
