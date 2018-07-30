#include "am_rx.hpp"

void am_rx(
    axis_t &axis_handler, //output
    axis_t &axis_net, //input
    s2mmCommand_t &axis_s2mmCommand, //output
    axis_t &axis_s2mm, //output
    axis_s2mmStatus_t &axis_s2mmStatus, //input

    //token RAM
    uint_16_t token_get, //input
    uint_1_t token_get_v, //input
    uint_8_t &record, //output

    //axis_handler release
    uint_1_t &release //output
){
    #pragma HLS INTERFACE axis port=axis_handler
    #pragma HLS INTERFACE axis port=axis_net
    #pragma HLS INTERFACE axis port=axis_s2mmCommand
	#pragma HLS INTERFACE axis port=axis_s2mm
	#pragma HLS INTERFACE axis port=axis_s2mmStatus
	#pragma HLS INTERFACE ap_ctrl_none port=return

    static enum state_t{st_header, st_AMheader, st_AMHandlerArgs,
        st_payloadSize, st_AMLongVector, st_AMdestination,
        st_AMpayload, st_AMLongStride, st_done, st_error} 
        currentState;

    axis_word_t axis_word_net;
    axis_word_8a_t axis_word_s2mmStatus;

    static gc_AMsrc_t AMsrc;
    static gc_AMwords_t AMwords;

    static gc_AMtype_t AMtype;
    static gc_AMargs_t AMargs;
    static gc_AMhandler_t AMhandler;

    static gc_payloadSize_t AMpayloadSize;
    static gc_dstVectorNum_t AMdstVectorNum;

    static gc_destinationLower_t AMdestinationLower;
    static gc_destinationUpper_t AMdestinationUpper;
    static gc_strideBlockSize_t AMstrideBlockSize;
    static gc_strideBlockNum_t AMstrideBlockNum;
    static gc_stride_t AMstride;

    static gc_vectorSize_t AMvectorSize[MAX_VECTOR_NUM];
    static gc_vectorDestLower_t AMvectorDestLower[MAX_VECTOR_NUM];
    static gc_vectorDestUpper_t AMvectorDestUpper[MAX_VECTOR_NUM];

    static gc_token_t AMtoken;

    switch(currentState){
        case st_header:{
            if(!axis_net.empty() && token_get_v == 1){
                axis_net.read(axis_word_net);
                AMsrc = axis_word_net.data(7,0);
                AMwords = axis_word_net.data(31,16);
                record = axis_word_net.data(7,0);
                currentState = st_AMheader;
                release = 1;
            }
            else{
                currentState = st_header;
            }
            break;
        }
        case st_AMheader:{
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                AMtype = axis_word_net.data(7,0);
                AMargs = axis_word_net.data(15,8);
                AMhandler = axis_word_net.data(31,16);
                if(isReplyAM(AMtype)){
                    AMtoken = -1;
                }
                else{
                    AMtoken = token_get;
                }
                
                if(isLongxAM(AMtype)){
                    release = 0;
                }
                axis_word_net.last = 0;
                axis_handler.write(axis_word_net);
                if(isShortAM(AMtype)){
                    if(axis_word_net.data(15,8) == 0){
                        currentState = st_done;
                    }
                    else{
                        currentState = st_AMHandlerArgs;
                    }
                }
                else{
                    currentState = st_payloadSize;
                }
            }
            else{
                currentState = st_AMheader;
            }
            break;
        }
        case st_AMHandlerArgs:{
            gc_AMargs_t argCount;
            axis_word_t axis_word;
            if(isShortAM(AMtype)){
                axis_word.data = AMtoken;
                axis_handler.write(axis_word);
            }
            for(argCount = 0; argCount < AMargs; argCount++){
                if(!axis_net.empty()){
                    axis_net.read(axis_word_net);
                    axis_handler.write(axis_word_net);
                }
            }
            if(isShortAM(AMtype)){
                currentState = st_done;
            }
            else {
                currentState = st_AMpayload;
            }
            break;
        }
        case st_payloadSize:{
            axis_word_t axis_word;
            axis_word.data = AMtoken;
            axis_handler.write(axis_word);
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                if(isLongVectoredAM(AMtype)){
                    AMpayloadSize = axis_word_net.data(31,8);
                    AMdstVectorNum = axis_word_net.data(7,4);
                    currentState = st_AMLongVector;
                }
                else{
                    AMpayloadSize = axis_word_net.data(31,0);
                    if(isLongxAM(AMtype)){
                        currentState = st_AMdestination;
                    }
                    else{ //medium message
                        if (AMargs == 0){
                            currentState = st_AMpayload;
                        }
                        else{
                            currentState = st_AMHandlerArgs;
                        }
                    }
                }
                axis_handler.write(axis_word_net);
            }
            else{
                currentState = st_payloadSize;
            }
            break;
        }
        case st_AMdestination:{
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                AMdestinationLower = axis_word_net.data(31,0);
                axis_handler.write(axis_word_net);
            }
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                AMdestinationUpper = axis_word_net.data(31,0);
                axis_handler.write(axis_word_net);
            }
            if(isLongAM(AMtype)){
                s2mmWriteCommand(axis_s2mmCommand, 0, 0, AMdestinationLower,
                    AMdestinationLower(1,0) != 0, 1, AMdestinationLower(1,0), 1, 
                    AMpayloadSize);
                if (AMargs == 0){
                    currentState = st_AMpayload;
                }
                else{
                    currentState = st_AMHandlerArgs;
                }
            }
            else{
                currentState = st_AMLongStride;
            }
            break;
        }
        case st_AMLongStride:{
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                AMstrideBlockSize = axis_word_net.data(31,0);
                axis_handler.write(axis_word_net);
            }
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                AMstrideBlockNum = axis_word_net.data(31,0);
                axis_handler.write(axis_word_net);
            }
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                AMstride = axis_word_net.data(31,0);
                axis_handler.write(axis_word_net);
            }
            s2mmWriteCommand(axis_s2mmCommand, 0, 0, AMdestinationLower,
                AMdestinationLower(1,0) != 0, AMstrideBlockNum != 1, 
                AMdestinationLower(1,0), 1, AMstrideBlockSize);
            if (AMargs == 0){
                currentState = st_AMpayload;
            }
            else{
                currentState = st_AMHandlerArgs;
            }
            break;
        }
        case st_AMLongVector:{
            gc_dstVectorNum_t i = 0;
            for(i = 0; i < AMdstVectorNum; i++){
                if(!axis_net.empty()){
                    axis_net.read(axis_word_net);
                    AMvectorSize[i] = axis_word_net.data(31,0);
                    axis_handler.write(axis_word_net);
                }
                if(!axis_net.empty()){
                    axis_net.read(axis_word_net);
                    AMvectorDestLower[i] = axis_word_net.data(31,0);
                    axis_handler.write(axis_word_net);
                }
                if(!axis_net.empty()){
                    axis_net.read(axis_word_net);
                    AMvectorDestUpper[i] = axis_word_net.data(31,0);
                    axis_handler.write(axis_word_net);
                }
            }
            s2mmWriteCommand(axis_s2mmCommand, 0, 0, AMvectorDestLower[0](31,0),
                AMvectorDestLower[0](1,0) != 0, AMdstVectorNum == 1, 
                AMvectorDestLower[0](1,0), 1, AMvectorSize[0]);
            if (AMargs == 0){
                currentState = st_AMpayload;
            }
            else{
                currentState = st_AMHandlerArgs;
            }
            break;
        }
        case st_AMpayload:{
            gc_payloadSize_t i = 0;
            gc_payloadSize_t writeCount = 0;
            gc_destinationLower_t strideDest = AMdestinationLower + AMstride;
            gc_strideBlockNum_t strideCount = 0;
            gc_dstVectorNum_t vectorCount = 0;
            while(i < AMpayloadSize){
                while(axis_net.empty()){
                    //busy loop
                }
                axis_net.read(axis_word_net);
                writeCount++;
                i++;
                if(isMediumAM(AMtype)){
                    axis_handler.write(axis_word_net);
                }
                else{
                    if(isLongAM(AMtype)){
                        if(!axis_s2mm.full()){
                            axis_s2mm.write(axis_word_net);
                        }
                    }
                    else if(isLongStridedAM(AMtype)){
                        if(writeCount < AMstrideBlockSize){
                            if(!axis_s2mm.full()){
                                axis_s2mm.write(axis_word_net);
                            }
                        }
                        else if(writeCount == AMstrideBlockSize){
                            if(!axis_s2mm.full()){
                                axis_word_net.last = 1;
                                axis_s2mm.write(axis_word_net);
                            }
                        }
                        else{
                            s2mmWriteCommand(axis_s2mmCommand, 0, 0, 
                                strideDest, 
                                strideDest(1,0) != 0, 
                                strideCount != AMstrideBlockNum, 
                                strideDest(1,0), 1, 
                                AMstrideBlockSize);
                            strideDest += AMstride;
                            if(!axis_s2mm.full()){
                                axis_s2mm.write(axis_word_net);
                            }
                            writeCount = 1;
                        }
                    }
                    else{
                        if(writeCount < AMvectorSize[vectorCount]){
                            if(!axis_s2mm.full()){
                                axis_s2mm.write(axis_word_net);
                            }
                        }
                        else if(writeCount == AMvectorSize[vectorCount]){
                            if(!axis_s2mm.full()){
                                axis_word_net.last = 1;
                                axis_s2mm.write(axis_word_net);
                            }
                        }
                        else{
                            vectorCount++;
                            s2mmWriteCommand(axis_s2mmCommand, 0, 0, 
                                AMvectorDestLower[vectorCount], //address
                                AMvectorDestLower[vectorCount](1,0) != 0, //drr 
                                AMdstVectorNum != vectorCount, //eof
                                AMvectorDestLower[vectorCount](1,0), 1, //dsa, type
                                AMvectorSize[vectorCount]);
                            if(!axis_s2mm.full()){
                                axis_s2mm.write(axis_word_net);
                            }
                            writeCount = 1;
                        }
                    }
                }
            }
            currentState = st_done;
            break;
        }
        case st_done:{
            if(isShortAM(AMtype) && AMargs == 0){
                axis_word_t axis_word;
                axis_word.data = AMtoken;
                axis_word.last = 1;
                axis_handler.write(axis_word);
                currentState = st_header;
            }
            else if(isLongxAM(AMtype)){
                if(axis_s2mmStatus.empty()){
                    currentState = st_done;
                }
                else{
                    axis_s2mmStatus.read(axis_word_s2mmStatus);
                    release = 1;
                    currentState = st_header;
                }
            }
            else{
                currentState = st_header;
            }
            break;
        }
        case st_error:{
            currentState = st_error;
            break;
        }
        default:{
            currentState = st_error;
            break;
        }
    }

}

inline void s2mmWriteCommand(
    s2mmCommand_t &axis_s2mmCommand, //output
    uint_4_t reserved,
    uint_4_t tag,
    uint_32_t address,
    uint_1_t ddr,
    uint_1_t eof,
    uint_6_t dsa,
    uint_1_t type,
    uint_23_t btt
){
    s2mmCommand_word_t axis_word_s2mmCommand;
    axis_word_s2mmCommand.data(40+GC_ADDR_WIDTH-1,36+GC_ADDR_WIDTH) = reserved;
    axis_word_s2mmCommand.data(36+GC_ADDR_WIDTH-1,32+GC_ADDR_WIDTH) = tag;
    axis_word_s2mmCommand.data(32+GC_ADDR_WIDTH-1,32) = address;
    axis_word_s2mmCommand.data[31] = ddr;
    axis_word_s2mmCommand.data[30] = eof;
    axis_word_s2mmCommand.data(29, 24) = dsa;
    axis_word_s2mmCommand.data[23] = type;
    axis_word_s2mmCommand.data(22,0) = btt;
    if(!axis_s2mmCommand.full()){
        axis_s2mmCommand.write(axis_word_s2mmCommand);
    }
}
