#include "am_tx.hpp"

void am_tx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_kernel, //input
    axis_t &axis_net, //output
    dataMoverCommand_t &axis_mm2sCommand, //output
    axis_t &axis_mm2s, //input
    dataMoverStatus_t &axis_mm2sStatus, //input
    uint_1_t &release //output
){
    #pragma HLS INTERFACE axis port=axis_kernel
    #pragma HLS INTERFACE axis port=axis_net
    #pragma HLS INTERFACE axis port=axis_mm2sCommand
	#pragma HLS INTERFACE axis port=axis_mm2s
	#pragma HLS INTERFACE axis port=axis_mm2sStatus
	#pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_none port=release
    #ifdef DEBUG
    #pragma HLS INTERFACE ap_none port=dbg_currentState
    #endif

    static state_t currentState = st_header;
    axis_word_t axis_word;

    static gc_AMsrc_t AMsrc;
    static gc_AMToken_t AMToken;
    // static gc_AMwords_t AMwords;

    static gc_AMtype_t AMtype;
    static gc_AMargs_t AMargs;
    static gc_AMhandler_t AMhandler;

    static gc_payloadSize_t AMpayloadSize;
    static gc_dstVectorNum_t AMdstVectorNum;
    static gc_srcVectorNum_t AMsrcVectorNum;

    // static gc_destinationLower_t AMsourceLower;
    // static gc_destinationUpper_t AMsourceUpper;
    // static gc_destinationLower_t AMdestinationLower;
    // static gc_destinationUpper_t AMdestinationUpper;
    static gc_destination_t AMsrcAddr;
    static gc_destination_t AMdstAddr;
    // static gc_strideBlockSize_t AMstrideBlockSize;
    // static gc_strideBlockNum_t AMstrideBlockNum;
    // static gc_stride_t AMstride;

    static gc_vectorSize_t AMvectorSize[MAX_VECTOR_NUM];
    // static gc_vectorDestLower_t AMvectorDestLower[MAX_VECTOR_NUM];
    // static gc_vectorDestUpper_t AMvectorDestUpper[MAX_VECTOR_NUM];
    static gc_destination_t AMvectorDest[MAX_VECTOR_NUM];

    static uint_1_t bufferRelease;

    switch(currentState){
        case st_header:{
            if(!axis_kernel.empty()){
                axis_kernel.read(axis_word);

                AMpayloadSize = axis_word.data(AM_PAYLOAD_SIZE);
                AMhandler = axis_word.data(AM_HANDLER);
                AMtype = axis_word.data(AM_TYPE);
                AMargs = axis_word.data(AM_HANDLER_ARGS);                

                axis_word.data(AM_TYPE) = AMtype & 0xEF; //mask out FIFO bit

                axis_net.write(axis_word);
                if((isLongxAM(AMtype) || isMediumAM(AMtype)) && 
                    !isDataFromFIFO(AMtype)){
                    bufferRelease = 0;
                }
                else{
                    bufferRelease = 1;
                }
                if(isLongStridedAM(AMtype)){
                    currentState = st_AMLongStride;
                }
                else if(isLongVectoredAM(AMtype)){
                    currentState = st_AMLongVector;
                }
                else{
                    currentState = st_AMToken;
                } 
            }
            else{
                currentState = st_header;
            }
            break;
        }
        case st_AMToken:{
            // if(!axis_kernel.empty()){
                axis_kernel.read(axis_word);
                axis_net.write(axis_word);
                AMToken = axis_word.data(AM_TOKEN);
                if(isShortAM(AMtype)){
                    currentState = AMargs == 0 ? st_done : st_AMHandlerArgs;
                }
                else if(isLongAM(AMtype)){
                    currentState = isDataFromFIFO(AMtype) ? st_AMdestination : st_AMsource;
                }
                else{
                    if(isDataFromFIFO(AMtype)){
                        currentState = AMargs == 0 ? st_AMpayload : st_AMHandlerArgs;
                    }
                    else{
                        currentState = st_AMsource;
                    }
                }
            // }
            break;
        }
        case st_AMHandlerArgs:{
            gc_AMargs_t argCount;
            uint_1_t enableLast = AMpayloadSize == 0;
            for(argCount = 0; argCount < AMargs; argCount++){
                // if(!axis_kernel.empty()){
                    axis_kernel.read(axis_word);
                    axis_word.last = enableLast;
                    axis_net.write(axis_word);
                // }
            }
            currentState = isShortAM(AMtype) ? st_done : st_AMpayload;
            break;
        }
        // case st_payloadSize:{
        //     if(!axis_kernel.empty()){
        //         axis_kernel.read(axis_word);
        //         if(isLongVectoredAM(AMtype)){
        //             AMpayloadSize = axis_word.data(31,8);
        //             AMdstVectorNum = axis_word.data(7,4);
        //             AMsrcVectorNum = axis_word.data(3,0);
        //             currentState = st_AMLongVector;
        //         }
        //         else{
        //             AMpayloadSize = axis_word.data(31,0);
        //             if(isLongxAM(AMtype)){
        //                 if(isDataFromFIFO(AMtype)){
        //                     currentState = st_AMdestination;
        //                 }
        //                 else{
        //                     currentState = st_AMsource;
        //                 }
        //             }
        //             else{ //medium message
        //                 if(isDataFromFIFO(AMtype)){
        //                     if (AMargs == 0){
        //                         currentState = st_AMpayload;
        //                     }
        //                     else{
        //                         currentState = st_AMHandlerArgs;
        //                     }
        //                 }
        //                 else{
        //                     currentState = st_AMsource;
        //                 }
        //             }
        //         }
        //     }
        //     else{
        //         currentState = st_payloadSize;
        //     }
        //     break;
        // }
        case st_AMsource:{
            // if(!axis_kernel.empty()){
                axis_kernel.read(axis_word);
                AMsrcAddr = axis_word.data;
            // }
            word_t address = AMsrcAddr(GC_ADDR_WIDTH-1,0);
            dataMoverWriteCommand(axis_mm2sCommand, 0, 0, address,
                address(1,0) != 0, 1, 
                address(1,0), 1, AMpayloadSize * GC_DATA_BYTES);
            if(isMediumAM(AMtype)){
                currentState = AMargs == 0 ? st_AMpayload : st_AMHandlerArgs;
            }
            else{
                currentState = st_AMdestination;
            }
            break;
        }
        // case st_AMLongStrideRead:{
        //     gc_stride_t srcStride;
        //     gc_strideBlockSize_t srcBlockSize;
        //     gc_strideBlockNum_t srcBlockNum;
        //     if(!axis_kernel.empty()){ //get stride
        //         axis_kernel.read(axis_word);
        //         srcStride = axis_word.data;
        //     }
        //     if(!axis_kernel.empty()){ //get block size
        //         axis_kernel.read(axis_word);
        //         srcBlockSize = axis_word.data;
        //     }
        //     if(!axis_kernel.empty()){ //get block num
        //         axis_kernel.read(axis_word);
        //         srcBlockNum = axis_word.data;
        //     }
        //     gc_strideBlockNum_t i;
        //     word_t address = AMsourceLower(31,0);
        //     for(i = 0; i < srcBlockNum; i++){
        //         word_t address = AMsourceLower(31,0);
        //         dataMoverWriteCommand(axis_mm2sCommand, 0, 0, address,
        //             address(1,0) != 0, i == srcBlockNum - 1, 
        //             address(1,0), 1, srcBlockSize);
        //         address += srcStride;
        //     }
        //     currentState = st_AMdestination;
        //     break;
        // }
        case st_AMdestination:{
            // if(!axis_kernel.empty()){ //read address
                axis_kernel.read(axis_word);
                axis_net.write(axis_word);
            // }
            currentState = AMargs == 0 ? st_AMpayload : st_AMHandlerArgs;
            break;
        }
        case st_AMLongStride:{
            gc_stride_t srcStride;
            gc_strideBlockSize_t srcBlockSize;
            gc_strideBlockNum_t srcBlockNum;
            // if(!axis_kernel.empty()){ //get metadata
                axis_kernel.read(axis_word);
                srcStride = axis_word.data(15,0);
                srcBlockSize = axis_word.data(27,16);
                srcBlockNum = axis_word.data(39,28);
            // }
            // if(!axis_kernel.empty()){ //get source address
                axis_kernel.read(axis_word);
                AMsrcAddr = axis_word.data;
            // }
            // if(!axis_kernel.empty()){ //get destination metadata
                axis_kernel.read(axis_word);
                axis_net.write(axis_word);
            // }
            // if(!axis_kernel.empty()){ //get destination address
                axis_kernel.read(axis_word);
                axis_net.write(axis_word);
            // }
            gc_strideBlockNum_t i;
            word_t address = AMsrcAddr(GC_ADDR_WIDTH-1,0);
            for(i = 0; i < srcBlockNum; i++){
                dataMoverWriteCommand(axis_mm2sCommand, 0, 0, address,
                    address(1,0) != 0, i == srcBlockNum - 1, 
                    address(1,0), 1, srcBlockSize * GC_DATA_BYTES);
                address += srcStride;
            }
            currentState = AMargs == 0 ? st_AMpayload : st_AMHandlerArgs;
            break;
        }
        case st_AMLongVector:{
            gc_dstVectorNum_t i = 0;
            // if(!axis_kernel.empty()){
                axis_kernel.read(axis_word);
                AMsrcVectorNum = axis_word.data(3,0);
                AMdstVectorNum = axis_word.data(7,4);
                AMvectorSize[0] = axis_word.data(19,8);
                axis_word.data(3,0) = 0;
                axis_word.data(19,8) = 0;
                axis_net.write(axis_word);
            // }
            // if(!axis_kernel.empty()){ //read src address
                axis_kernel.read(axis_word);
                AMvectorDest[0] = axis_word.data;
            // }
            word_t address = AMvectorDest[0](GC_ADDR_WIDTH-1,0);
            dataMoverWriteCommand(axis_mm2sCommand, 0, 0, address,
                address(1,0) != 0, AMsrcVectorNum == 1, 
                address(1,0), 1, AMvectorSize[0] * GC_DATA_BYTES);
            // if(!axis_kernel.empty()){ //read dst address
                axis_kernel.read(axis_word);
                axis_net.write(axis_word);
            // }
            for(i = 1; i < AMsrcVectorNum; i++){
                // if(!axis_kernel.empty()){ //read src size
                    axis_kernel.read(axis_word);
                    AMvectorSize[i] = axis_word.data;
                // }
                // if(!axis_kernel.empty()){ //read src address
                    axis_kernel.read(axis_word);
                    AMvectorDest[i] = axis_word.data;
                // }
                word_t address = AMvectorDest[i](GC_ADDR_WIDTH-1,0);
                dataMoverWriteCommand(axis_mm2sCommand, 0, 0, address,
                    address(1,0) != 0, i == AMsrcVectorNum - 1, 
                    address(1,0), 1, AMvectorSize[i] * GC_DATA_BYTES);
            }
            for(i = 1; i < AMdstVectorNum; i++){
                // if(!axis_kernel.empty()){ //read dst size
                    axis_kernel.read(axis_word);
                    axis_net.write(axis_word);
                // }
                // if(!axis_kernel.empty()){ //read dst address
                    axis_kernel.read(axis_word);
                    axis_net.write(axis_word);
                // }
            }
            currentState = AMargs == 0 ? st_AMpayload : st_AMHandlerArgs;
            break;
        }
        case st_AMpayload:{
            gc_payloadSize_t i = 0;
            for(i = 0; i < AMpayloadSize-1; i++){
            // while(i < AMpayloadSize){
                if(isDataFromFIFO(AMtype))
                    axis_kernel.read(axis_word);
                else
                    axis_mm2s.read(axis_word);
                // i++;
                axis_net.write(axis_word);
            }
            if(isDataFromFIFO(AMtype))
                axis_kernel.read(axis_word);
            else
                axis_mm2s.read(axis_word);
            axis_word.last = 1;
            axis_net.write(axis_word);
            currentState = st_done;
            break;
        }
        case st_done:{
            bufferRelease = 1;
            currentState = st_header;
            break;
        }
    }

    release = bufferRelease == 0 ? 0 : 1;

    #ifdef DEBUG
    dbg_currentState = currentState;
    #endif

}

#ifdef DEBUG
std::string stateParse(int state){
    switch(state){
        CHECK_STATE("st_header", st_header, 0)
        CHECK_STATE("st_AMHandlerArgs", st_AMHandlerArgs, 1)
        CHECK_STATE("st_AMLongVector", st_AMLongVector, 2)
        CHECK_STATE("st_AMdestination", st_AMdestination, 3)
        CHECK_STATE("st_AMToken", st_AMToken, 4)
        CHECK_STATE("st_AMpayload", st_AMpayload, 5)
        CHECK_STATE("st_AMLongStride", st_AMLongStride, 6)
        CHECK_STATE("st_done", st_done, 7)
        CHECK_STATE("st_AMsource", st_AMsource, 8)
        default: {
            std::stringstream sstm;
            sstm << "Unknown State " << state;
            std::string result = sstm.str();
            return result;
        }
    }
}
#endif
