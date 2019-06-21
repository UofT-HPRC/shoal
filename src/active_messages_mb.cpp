#include "active_messages.hpp"
#include "am_globals.hpp"

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

#endif // ENABLE_NETWORK

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
