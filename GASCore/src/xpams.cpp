#include "xpams.hpp"

void xpams(
    axis_t &axis_handler, //input
    axis_t &axis_handler_return, //output
    //connection to tx
    //connection to tx
    uint_32_t * ctrl
){
    #pragma HLS INTERFACE axis port=axis_handler
    #pragma HLS INTERFACE axis port=axis_handler_return
    #pragma HLS INTERFACE s_axilite port=ctrl
	#pragma HLS INTERFACE ap_ctrl_none port=return

    axis_word_t axis_word;

    static enum state_t{st_idle, st_AMheader, st_token, st_AMHandlerArgs,
        st_payloadSize, st_AMLongVector, st_AMdestination, st_replyHandler,
        st_codeLoad, st_directReply, st_AMpayload, st_AMLongStride, st_done, 
        st_error} currentState;

    static gc_AMargs_t AMargs;
    static gc_AMtype_t AMtype;
    static gc_AMhandler_t AMhandler;

    static gc_token_t AMtoken;

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

    switch(currentState){
        case st_idle:{
            if(!axis_handler.empty()){
                currentState = st_AMheader;
            }
            break;
        }
        case st_AMheader:{
            if(!axis_handler.empty()){
                axis_handler.read(axis_word);
                AMtype = axis_word.data(7,0);
                AMargs = axis_word.data(15,8);
                AMhandler = axis_word.data(31,16);
                currentState = st_token;
                break;
            }
        }
        case st_token:{
            if(!axis_handler.empty()){
                axis_handler.read(axis_word);
                AMtoken = axis_word.data;
                if(isShortAM(AMtype)){
                    if(isDirectReply(AMhandler)){
                        currentState = st_directReply;
                    }
                    else if(isReply(AMhandler)){
                        currentState = st_replyHandler;
                    }
                    else if(AMargs != 0){
                        currentState = st_AMHandlerArgs;
                    }
                    else{
                        currentState = st_done;
                    }
                }
                else{
                    currentState = st_payloadSize;
                }
                break;
            }
        }
        case st_AMHandlerArgs:{
            gc_AMargs_t argCount;
            for(argCount = 0; argCount < AMargs; argCount++){
                if(!axis_handler.empty()){
                    axis_handler.read(axis_word);
                }
            }
            if(isMediumAM(AMtype)){
                currentState = st_AMpayload;
            }
            else {
                currentState = st_done;
            }
            break;
        }
        case st_payloadSize:{
            axis_word.data = AMtoken;
            if(!axis_handler.empty()){
                axis_handler.read(axis_word);
                if(isLongVectoredAM(AMtype)){
                    AMpayloadSize = axis_word.data(31,8);
                    AMdstVectorNum = axis_word.data(7,4);
                    currentState = st_AMLongVector;
                }
                else{
                    AMpayloadSize = axis_word.data(31,0);
                    if(isLongxAM(AMtype)){
                        currentState = st_AMdestination;
                    }
                    else{ //medium message
                        if(isDirectReply(AMhandler)){
                            currentState = st_directReply;
                        }
                        else if(isReply(AMhandler)){
                            currentState = st_replyHandler;
                        }
                        else if(isCodeLoad(AMhandler)){
                            currentState = st_codeLoad;
                        }
                        else if (AMargs != 0){
                            currentState = st_AMHandlerArgs;
                        }
                        else{
                            currentState = st_AMpayload;
                        }
                    }
                }
            }
            else{
                currentState = st_payloadSize;
            }
            break;
        }
        case st_directReply:{
            
        }
        case st_replyHandler:{

        }
        case st_codeLoad:{

        }
        case st_AMdestination:{
            if(!axis_handler.empty()){
                axis_handler.read(axis_word);
                AMdestinationLower = axis_word.data(31,0);
            }
            if(!axis_handler.empty()){
                axis_handler.read(axis_word);
                AMdestinationUpper = axis_word.data(31,0);
            }
            if(isLongAM(AMtype)){
                if(isDirectReply(AMhandler)){
                    currentState = st_directReply;
                }
                else if(isReply(AMhandler)){
                    currentState = st_replyHandler;
                }
                else if(isCodeLoad(AMhandler)){
                    currentState = st_codeLoad;
                }
                else if (AMargs != 0){
                    currentState = st_AMHandlerArgs;
                }
                else{
                    currentState = st_done;
                }
            }
            else{
                currentState = st_AMLongStride;
            }
            break;
        }
        case st_AMLongStride:{
            if(!axis_handler.empty()){
                axis_handler.read(axis_word);
                AMstrideBlockSize = axis_word.data(31,0);
            }
            if(!axis_handler.empty()){
                axis_handler.read(axis_word);
                AMstrideBlockNum = axis_word.data(31,0);
            }
            if(!axis_handler.empty()){
                axis_handler.read(axis_word);
                AMstride = axis_word.data(31,0);
            }
            if(isDirectReply(AMhandler)){
                currentState = st_directReply;
            }
            else if(isReply(AMhandler)){
                currentState = st_replyHandler;
            }
            else if(isCodeLoad(AMhandler)){
                currentState = st_codeLoad;
            }
            else if (AMargs != 0){
                currentState = st_AMHandlerArgs;
            }
            else{
                currentState = st_done;
            }
            break;
        }
        case st_AMLongVector:{
            gc_dstVectorNum_t i = 0;
            for(i = 0; i < AMdstVectorNum; i++){
                if(!axis_handler.empty()){
                    axis_handler.read(axis_word);
                    AMvectorSize[i] = axis_word.data(31,0);
                }
                if(!axis_handler.empty()){
                    axis_handler.read(axis_word);
                    AMvectorDestLower[i] = axis_word.data(31,0);
                }
                if(!axis_handler.empty()){
                    axis_handler.read(axis_word);
                    AMvectorDestUpper[i] = axis_word.data(31,0);
                }
            }
           if(isDirectReply(AMhandler)){
                currentState = st_directReply;
            }
            else if(isReply(AMhandler)){
                currentState = st_replyHandler;
            }
            else if(isCodeLoad(AMhandler)){
                currentState = st_codeLoad;
            }
            else if (AMargs != 0){
                currentState = st_AMHandlerArgs;
            }
            else{
                currentState = st_done;
            }
            break;
        }
        case st_AMpayload:{
            gc_payloadSize_t i = 0;
            while(i < AMpayloadSize){
                while(axis_handler.empty()){
                    //busy loop
                }
                axis_handler.read(axis_word);
                i++;
            }
            currentState = st_done;
            break;
        }
        case st_done:{
            currentState = st_idle;
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

bool isDirectReply(gc_AMhandler_t handler){
    return handler == DIRECT_REPLY;
}

bool isReply(gc_AMhandler_t handler){
    return handler == REPLY_HANDLER;
}

bool isCodeLoad(gc_AMhandler_t handler){
    return handler == CODE_LOAD;
}