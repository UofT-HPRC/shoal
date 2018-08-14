#include "am_rx.hpp"

void am_rx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_handler, //output
    axis_t &axis_net, //input
    dataMoverCommand_t &axis_s2mmCommand, //output
    axis_t &axis_s2mm, //output
    dataMoverStatus_t &axis_s2mmStatus, //input

    //axis_handler release
    uint_1_t &release //output
){
    #pragma HLS INTERFACE axis port=axis_handler
    #pragma HLS INTERFACE axis port=axis_net
    #pragma HLS INTERFACE axis port=axis_s2mmCommand
	#pragma HLS INTERFACE axis port=axis_s2mm
	#pragma HLS INTERFACE axis port=axis_s2mmStatus
	#pragma HLS INTERFACE ap_ctrl_none port=return

    axis_word_t axis_word_net;
    axis_word_8a_t axis_word_s2mmStatus;

    static gc_AMsrc_t AMsrc;
    static gc_AMToken_t AMToken;
    static gc_AMtype_t AMtype;
    static gc_AMargs_t AMargs;
    static gc_AMhandler_t AMhandler;

    static gc_payloadSize_t AMpayloadSize;
    static gc_dstVectorNum_t AMdstVectorNum;

    static gc_destination_t AMdestination;
    static gc_strideBlockSize_t AMstrideBlockSize;
    static gc_strideBlockNum_t AMstrideBlockNum;
    static gc_stride_t AMstride;

    static gc_vectorSize_t AMvectorSize[MAX_VECTOR_NUM];
    static gc_destination_t AMvectorDest[MAX_VECTOR_NUM];

    switch(currentState){
        case st_header:{
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                AMsrc = axis_word_net.data(15,0);
                AMpayloadSize = axis_word_net.data(43,32);
                AMhandler = axis_word_net.data(47,44);
                AMtype = axis_word_net.data(55,48);
                AMargs = axis_word_net.data(63,56);
                
                axis_handler.write(axis_word_net);
                release = !isLongxAM(AMtype);

                
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
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                axis_handler.write(axis_word_net);
                AMToken = axis_word_net.data(GC_DATA_WIDTH-1,GC_DATA_WIDTH-24);
                if(isShortAM(AMtype)){
                    currentState = AMargs == 0 ? st_done : st_AMHandlerArgs;
                }
                else if(isMediumAM(AMtype)){
                    currentState = AMargs == 0 ? st_AMpayload : st_AMHandlerArgs;
                }
                else{
                    currentState = st_AMdestination;
                }
            }
            break;
        }
        case st_AMHandlerArgs:{
            gc_AMargs_t argCount;
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
        case st_AMdestination:{
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                AMdestination = axis_word_net.data(63,0);
                axis_handler.write(axis_word_net);

                gc_strideBlockSize_t payload = isLongAM(AMtype) ? AMpayloadSize : AMstrideBlockSize;
                addr_word_t address = AMdestination(GC_ADDR_WIDTH-1,0);
                dataMoverWriteCommand(axis_s2mmCommand, 0, 0, address,
                    address(1,0) != 0, 1, address(1,0), 1, payload*GC_DATA_BYTES);
                if (AMargs == 0){
                    currentState = st_AMpayload;
                }
                else{
                    currentState = st_AMHandlerArgs;
                }
                break;
            }            
        }
        case st_AMLongStride:{
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                axis_handler.write(axis_word_net);
                AMstride = axis_word_net.data(15,0);
                AMstrideBlockSize = axis_word_net.data(27,16);
                AMstrideBlockNum = axis_word_net.data(39,28);
                AMToken = axis_word_net.data(63,40);
                currentState = st_AMdestination;
            }
            break;
        }
        case st_AMLongVector:{
            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                AMdstVectorNum = axis_word_net.data(7,4);
                AMvectorSize[0] = axis_word_net.data(31,20);
                AMToken = axis_word_net.data(63,40);
                axis_handler.write(axis_word_net);
            }

            if(!axis_net.empty()){
                axis_net.read(axis_word_net);
                AMvectorDest[0] = axis_word_net.data(63,0);
            }

            gc_dstVectorNum_t i = 0;
            for(i = 1; i < AMdstVectorNum; i++){
                if(!axis_net.empty()){
                    axis_net.read(axis_word_net);
                    AMvectorSize[i] = axis_word_net.data(11,0);
                }
                if(!axis_net.empty()){
                    axis_net.read(axis_word_net);
                    AMvectorDest[i] = axis_word_net.data(63,0);
                }
            }
            dataMoverWriteCommand(axis_s2mmCommand, 0, 0, 
                AMvectorDest[0](GC_ADDR_WIDTH-1,0),
                AMvectorDest[0](1,0) != 0, 1, 
                AMvectorDest[0](1,0), 1, AMvectorSize[0]*GC_DATA_BYTES);
            
            currentState = AMargs == 0 ? st_AMpayload : st_AMHandlerArgs;
            break;
        }
        case st_AMpayload:{
            gc_payloadSize_t i = 0;
            gc_payloadSize_t writeCount = 0;
            gc_destination_t strideDest = AMdestination + AMstride;
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
                            dataMoverWriteCommand(axis_s2mmCommand, 0, 0, 
                                strideDest, strideDest(1,0) != 0, 1, 
                                strideDest(1,0), 1, AMstrideBlockSize * GC_DATA_BYTES);
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
                            dataMoverWriteCommand(axis_s2mmCommand, 0, 0, 
                                AMvectorDest[vectorCount], //address
                                AMvectorDest[vectorCount](1,0) != 0, //ddr 
                                1, //eof
                                AMvectorDest[vectorCount](1,0), 1, //dsa, type
                                AMvectorSize[vectorCount]*GC_DATA_BYTES);
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

    #ifdef DEBUG
    dbg_currentState = currentState;
    #endif

}

std::string stateParse(int state){
    switch(state){
        case 0: {
            if(st_header == 0)
                return "st_header";
            else
                return "State Mismatch";
        }
        case 1: {
            if(st_AMHandlerArgs == 1)
                return "st_AMHandlerArgs";
            else
                return "State Mismatch";
        }
        case 2: {
            if(st_AMLongVector == 2)
                return "st_AMLongVector";
            else
                return "State Mismatch";
        }
        case 3: {
            if(st_AMdestination == 3)
                return "st_AMdestination";
            else
                return "State Mismatch";
        }
        case 4: {
            if(st_AMToken == 4)
                return "st_AMToken";
            else
                return "State Mismatch";
        }
        case 5: {
            if(st_AMpayload == 5)
                return "st_AMpayload";
            else
                return "State Mismatch";
        }
        case 6: {
            if(st_AMLongStride == 5)
                return "st_AMLongStride";
            else
                return "State Mismatch";
        }
        case 7: {
            if(st_done == 5)
                return "st_done";
            else
                return "State Mismatch";
        }
        case 8: {
            if(st_error == 5)
                return "st_error";
            else
                return "State Mismatch";
        }
        default: return "Unknown State";
    }
}