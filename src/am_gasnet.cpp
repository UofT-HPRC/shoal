#include "am_globals.hpp"
#include "am_gasnet.hpp"
#include "global_utilities.hpp"
#include "config.hpp"

#define CPU
#include "active_messages.hpp"

void am_tx(galapagos::stream <word_t> * in, galapagos::stream <word_t> * out){
    gc_AMsrc_t AMsrc;
    gc_AMdst_t AMdst;
    gc_AMToken_t AMToken;
    // static gc_AMwords_t AMwords;

    gc_AMtype_t AMtype;
    gc_AMargs_t AMargs;
    gc_AMhandler_t AMhandler;

    gc_payloadSize_t AMpayloadSize;
    gc_dstVectorNum_t AMdstVectorNum;
    gc_srcVectorNum_t AMsrcVectorNum;

    gc_strideBlockNum_t srcBlockNum;

    // static gc_destinationLower_t AMsourceLower;
    // static gc_destinationUpper_t AMsourceUpper;
    // static gc_destinationLower_t AMdestinationLower;
    // static gc_destinationUpper_t AMdestinationUpper;
    gc_destination_t AMsrcAddr;
    gc_destination_t AMdstAddr;
    // static gc_strideBlockSize_t AMstrideBlockSize;
    // static gc_strideBlockNum_t AMsstrideBlockNum;
    // static gc_stride_t AMstride;

    gc_vectorSize_t AMvectorSize[MAX_VECTOR_NUM];
    // static gc_vectorDestLower_t AMvectorDestLower[MAX_VECTOR_NUM];
    // static gc_vectorDestUpper_t AMvectorDestUpper[MAX_VECTOR_NUM];
    gc_destination_t AMvectorDest[MAX_VECTOR_NUM];

    // static uint_1_t bufferRelease = 1;

    gc_stride_t srcStride;
    gc_strideBlockSize_t srcBlockSize;

    gc_dstVectorNum_t i = 0;

    word_t address;

    galapagos::stream_packet <word_t> axis_word;

    // switch(currentState){
    //     case st_header:{
            // if(!in->empty()){
    axis_word = in->read();

    #ifdef USE_ABS_PAYLOAD
    AMpayloadSize = hdextract(axis_word.data, AM_PAYLOAD_SIZE) - GC_DATA_BYTES;
    #else
    AMpayloadSize = hdextract(axis_word.data, AM_PAYLOAD_SIZE);
    #endif
    AMhandler = hdextract(axis_word.data, AM_HANDLER);
    AMtype = hdextract(axis_word.data, AM_TYPE);
    AMargs = hdextract(axis_word.data, AM_HANDLER_ARGS);
    AMdst = hdextract(axis_word.data, AM_DST);

    if (isReplyAM(AMtype) && (!isShortAM(AMtype))){
        ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
        out->write(axis_word);
        // axis_net.write(axis_word);
        axis_word = in->read();
        while(axis_word.last != 1){
            // axis_net.write(axis_word);
            ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
            out->write(axis_word);
            axis_word = in->read();
        }
        // axis_net.write(axis_word);
        ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
        out->write(axis_word);
    }
    else{

        axis_word.data = hdencode(axis_word.data, AM_TYPE, AMtype & 0xEF); //mask out FIFO bit

        // axis_net.write(axis_word);
        ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
        out->write(axis_word);
        // if((isLongxAM(AMtype) || isMediumAM(AMtype)) && 
        //     !isDataFromFIFO(AMtype)){
        //     // bufferRelease = 0;
        // }
        // else{
        //     // bufferRelease = 1;
        // }
        if(isLongStridedAM(AMtype)){
            // if(!in->empty()){ //get metadata
                axis_word = in->read();
                srcStride = hdextract(axis_word.data, AM_STRIDE_SIZE);
                srcBlockSize = hdextract(axis_word.data, AM_STRIDE_BLK_SIZE);
                srcBlockNum = hdextract(axis_word.data, AM_STRIDE_BLK_NUM);
            // }
            // if(!in->empty()){ //get source address
                axis_word = in->read();
                AMsrcAddr = axis_word.data;
            // }
            // if(!in->empty()){ //get destination metadata
                axis_word = in->read();
                // axis_net.write(axis_word);
                ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
                out->write(axis_word);
                #ifdef USE_ABS_PAYLOAD
                AMpayloadSize -= GC_DATA_BYTES;
                #endif
            // }
            // if(!in->empty()){ //get destination address
                axis_word = in->read();
                axis_word.last = 0;
                // axis_net.write(axis_word);
                ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
                out->write(axis_word);
                #ifdef USE_ABS_PAYLOAD
                AMpayloadSize -= GC_DATA_BYTES;
                #endif
            // }
            gc_strideBlockNum_t i;
            address = AMsrcAddr;
            // for(i = 0; i < srcBlockNum; i++){
                // dataMoverWriteCommand(axis_mm2sCommand, 0, 0, address,
                //     address(1,0) != 0, i == srcBlockNum - 1, 
                //     address(1,0), 1, srcBlockSize);
                // address += srcStride;
            // }
        }
        else if(isLongVectoredAM(AMtype)){
            
            // if(!in->empty()){
                axis_word = in->read();
                AMsrcVectorNum = hdextract(axis_word.data, AM_SRC_VECTOR_NUM);
                AMdstVectorNum = hdextract(axis_word.data, AM_DST_VECTOR_NUM);
                AMvectorSize[0] = hdextract(axis_word.data, AM_SRC_VECTOR_SIZE_HEAD);
                axis_word.data = hdencode(axis_word.data, AM_SRC_VECTOR_NUM, 0);
                axis_word.data = hdencode(axis_word.data, AM_SRC_VECTOR_SIZE_HEAD, 0);
                // axis_net.write(axis_word);
                ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
                out->write(axis_word);
                #ifdef USE_ABS_PAYLOAD
                AMpayloadSize -= GC_DATA_BYTES;
                #endif
            // }
            // if(!in->empty()){ //read src address
                axis_word = in->read();
                AMvectorDest[0] = axis_word.data;
            // }
            address = AMvectorDest[0];
            // dataMoverWriteCommand(axis_mm2sCommand, 0, 0, address,
            //     address(1,0) != 0, AMsrcVectorNum == 1, 
            //     address(1,0), 1, AMvectorSize[0]);
            // if(!in->empty()){ //read dst address
                axis_word = in->read();
                // axis_net.write(axis_word);
                ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
                out->write(axis_word);
                #ifdef USE_ABS_PAYLOAD
                AMpayloadSize -= GC_DATA_BYTES;
                #endif
            // }
            for(i = 1; i < AMsrcVectorNum; i++){
                // if(!in->empty()){ //read src size
                    axis_word = in->read();
                    AMvectorSize[i] = axis_word.data;
                // }
                // if(!in->empty()){ //read src address
                    axis_word = in->read();
                    AMvectorDest[i] = axis_word.data;
                // }
                // address = AMvectorDest[i];
                // dataMoverWriteCommand(axis_mm2sCommand, 0, 0, address,
                //     address(1,0) != 0, i == AMsrcVectorNum - 1, 
                //     address(1,0), 1, AMvectorSize[i]);
            }
            for(i = 1; i < AMdstVectorNum; i++){
                // if(!in->empty()){ //read dst size
                    axis_word = in->read();
                    // axis_net.write(axis_word);
                    ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
                    out->write(axis_word);
                    #ifdef USE_ABS_PAYLOAD
                    AMpayloadSize -= GC_DATA_BYTES;
                    #endif
                // }
                // if(!in->empty()){ //read dst address
                    axis_word = in->read();
                    axis_word.last = 0;
                    // axis_net.write(axis_word);
                    ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
                    out->write(axis_word);
                    #ifdef USE_ABS_PAYLOAD
                    AMpayloadSize -= GC_DATA_BYTES;
                    #endif
                // }
            }
        }
        else{
            axis_word = in->read();
            // axis_net.write(axis_word);
            ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
            out->write(axis_word);
            AMToken = hdextract(axis_word.data, AM_TOKEN);
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize -= GC_DATA_BYTES;
            #endif
            if(!isShortAM(AMtype) && !isDataFromFIFO(AMtype)){
                // if(!in->empty()){
                    axis_word = in->read();
                    AMsrcAddr = axis_word.data;
                // }
                // AMpayloadSize -= GC_DATA_BYTES; // ? this word isn't part of the final
                address = AMsrcAddr;
                #ifdef USE_ABS_PAYLOAD
                gc_payloadSize_t payload_size = AMpayloadSize - (AMargs*GC_DATA_BYTES);
                payload_size = payload_size - (isLongAM(AMtype) ? GC_DATA_BYTES : 0);
                #else
                gc_payloadSize_t payload_size = AMpayloadSize;
                #endif
                // dataMoverWriteCommand(axis_mm2sCommand, 0, 0, address,
                //     address(1,0) != 0, 1, 
                //     address(1,0), 1, payload_size);
                if(!isMediumAM(AMtype)){
                    // read destination
                    axis_word = in->read();
                    ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
                    out->write(axis_word);
                }
            }
            else if(isLongAM(AMtype)){
                // read destination
                axis_word = in->read();
                ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
                out->write(axis_word);
            }
        }

        // check for handler
        gc_AMargs_t argCount;
        // uint_1_t enableLast = AMpayloadSize == 0;
        for(argCount = 0; argCount < AMargs; argCount++){
            // if(!in->empty()){
                axis_word = in->read();
                // axis_word.last = enableLast;
                // axis_net.write(axis_word);
                ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
                out->write(axis_word);
                AMpayloadSize -= GC_DATA_BYTES;
            // }
        }

        // check for payload
        if (!isShortAM(AMtype)){
            gc_payloadSize_t i = 0;
            gc_payloadSize_t payload_1 = AMpayloadSize-GC_DATA_BYTES;
            int j = 0;
            int k = 0;
            for(i = 0; i < payload_1; i+=GC_DATA_BYTES){
            // while(i < AMpayloadSize){
                if(isDataFromFIFO(AMtype))
                    axis_word = in->read();
                else{
                    if(isLongStridedAM(AMtype)){
                        axis_word.data = *((word_t*) &gasnet_shared_mem[address]);
                        address += GC_DATA_BYTES;
                        j+= GC_DATA_BYTES;
                        if(j == srcBlockSize){
                            k++;
                            address = AMsrcAddr + (k*srcStride);
                            j = 0;
                        }
                    } else if(isLongVectoredAM(AMtype)){
                        axis_word.data = *((word_t*) &gasnet_shared_mem[address]);
                        address += GC_DATA_BYTES;
                        j += GC_DATA_BYTES;
                        if (j == AMvectorSize[k]){
                            k++;
                            j = 0;
                            address = AMvectorDest[k];
                        }
                    } else {
                        axis_word.data = *((word_t*) &gasnet_shared_mem[address]);
                        address += GC_DATA_BYTES;
                    }
                }
                    // axis_mm2s.read(axis_word);
                axis_word.last = 0;
                // i+=GC_DATA_BYTES;
                // axis_net.write(axis_word);
                ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
                out->write(axis_word);
            }
            if(isDataFromFIFO(AMtype))
                axis_word = in->read();
            else{
                if(isLongStridedAM(AMtype)){
                    axis_word.data = *((word_t*) &gasnet_shared_mem[address]);
                    address += GC_DATA_BYTES;
                    j+= GC_DATA_BYTES;
                    if(j == srcBlockSize){
                        k++;
                        address = AMsrcAddr + (k*srcStride);
                        j = 0;
                    }
                } else if(isLongVectoredAM(AMtype)){
                    axis_word.data = *((word_t*) &gasnet_shared_mem[address]);
                    address += GC_DATA_BYTES;
                    j += GC_DATA_BYTES;
                    if (j == AMvectorSize[k]){
                        k++;
                        j = 0;
                        address = AMvectorDest[k];
                    }
                } else {
                    axis_word.data = *((word_t*) &gasnet_shared_mem[address]);
                    address += GC_DATA_BYTES;
                }
            }
            axis_word.last = 1;
            // axis_net.write(axis_word);
            ATOMIC_ACTION(printWord("   Writing to network ", axis_word));
            out->write(axis_word);
        }                    
    }
}

void xpams_rx(galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out, galapagos::stream <word_t> * to_kernel){
    gc_AMsrc_t AMsrc;
    gc_AMdst_t AMdst;
    gc_AMToken_t AMToken;

    gc_payloadSize_t AMpayloadSize;

    gc_AMargs_t AMargs;
    gc_AMtype_t AMtype;
    gc_AMhandler_t AMhandler;

    galapagos::stream_packet <word_t> axis_word;

    axis_word = in->read();
    ATOMIC_ACTION(printWord("   Reading in xpams 1 - ", axis_word));
    AMsrc = hdextract(axis_word.data, AM_SRC);
    AMdst = hdextract(axis_word.data, AM_DST);
    AMpayloadSize = hdextract(axis_word.data, AM_PAYLOAD_SIZE);
    AMhandler = hdextract(axis_word.data, AM_HANDLER);
    AMtype = hdextract(axis_word.data, AM_TYPE);
    AMargs = hdextract(axis_word.data, AM_HANDLER_ARGS);
    if (isReplyAM(AMtype) && !isShortAM(AMtype)){
        axis_word.data = hdencode(axis_word.data, AM_TYPE, (AMtype & (~AM_REPLY)) + AM_ASYNC);
        axis_word.data = hdencode(axis_word.data, AM_SRC, AMdst);
        axis_word.data = hdencode(axis_word.data, AM_DST, AMsrc);
        ATOMIC_ACTION(printWord("   Writing to loopback 1 - ", axis_word));
        out->write(axis_word);
    }

    axis_word = in->read(); //read token
    ATOMIC_ACTION(printWord("   Reading in xpams 2 - ", axis_word));
    AMToken = hdextract(axis_word.data, AM_TOKEN);
    if(isReplyAM(AMtype) && isShortAM(AMtype)){
        axis_word.data = 0;
        axis_word.data = hdencode(axis_word.data, AM_REPLY_TYPE, AMtype);
        axis_word.data = hdencode(axis_word.data, AM_REPLY_SRC, AMsrc);
        #ifdef USE_ABS_PAYLOAD
        axis_word.data = hdencode(axis_word.data, AM_REPLY_PAYLOAD_SIZE, GC_DATA_BYTES);
        #else
        axis_word.data = hdencode(axis_word.data, AM_REPLY_PAYLOAD_SIZE, 0);
        #endif
        axis_word.data = hdencode(axis_word.data, AM_REPLY_TOKEN, AMToken);
        // hdextract(axis_word.data, AM_TOKEN) = AMToken;
        // hdextract(axis_word.data, 39,8) = 0; //TODO parameterize
        // hdextract(axis_word.data, AM_TYPE) = AM_SHORT + AM_REPLY;
        // axis_wordDest = assignWord(axis_word);
        // axis_wordDest.dest = AMdst;
        // ? Trying out not sending a word reply. Instead, just use a counter
        // ATOMIC_ACTION(printWord("   Writing to kernel - ", axis_word));
        // to_kernel->write(axis_word);
        // currentState = st_AMheader;
    }
    else if(isReplyAM(AMtype)){
        ATOMIC_ACTION(printWord("   Writing to loopback 2 - ", axis_word));
        out->write(axis_word);
        // axis_word = in->read();
        do{
            axis_word = in->read();
            ATOMIC_ACTION(printWord("   Writing to loopback 3 - ", axis_word));
            out->write(axis_word);
        } while(axis_word.last != 1);
        ATOMIC_ACTION(printWord("   Writing to loopback 4 - ", axis_word));
        out->write(axis_word);
        // currentState = st_AMheader;
    }
    else{
        // if (AMargs != 0){
        //     gc_AMargs_t i;
        //     for(i = 0; i < AMargs - 1; i++){
        //         axis_word = in->read();
        //         axis_wordNoKeep = assignWordtoNoKeep(axis_word);
        //         axis_handler.write(axis_wordNoKeep);
        //     }
        //     axis_word = in->read();
        //     axis_wordNoKeep = assignWordtoNoKeep(axis_word);
        //     axis_wordNoKeep.last = 1;
        //     axis_handler.write(axis_wordNoKeep);
        // }
        if(isMediumAM(AMtype)){
            axis_word.data = 0;
            axis_word.data = hdencode(axis_word.data, AM_TYPE, AMtype);
            axis_word.data = hdencode(axis_word.data, AM_SRC, AMsrc);
            #ifdef USE_ABS_PAYLOAD
            hdextract(axis_word.data, AM_DST) = AMpayloadSize - AMargs - GC_DATA_BYTES;
            #else
            axis_word.data = hdencode(axis_word.data, AM_DST, AMpayloadSize);
            #endif
            axis_word.data = hdencode(axis_word.data, AM_TOKEN, AMToken);
            // axis_wordDest = assignWord(axis_word);
            // axis_wordDest.dest = AMdst;
            ATOMIC_ACTION(printWord("   Writing to kernel - ", axis_word));
            to_kernel->write(axis_word);
            do{
                axis_word = in->read();
                // axis_wordDest = assignWord(axis_word);
                // axis_wordDest.dest = AMdst;
                ATOMIC_ACTION(printWord("   Writing to kernel - ", axis_word));
                to_kernel->write(axis_word);
            } while(!axis_word.last);
            if (!isAsyncAM(AMtype)){

            }
            //     currentState = st_AMheader;
            // else
            //     currentState = st_sendReplyHeader;
        }
        // else{
        //     if (isAsyncAM(AMtype) || isReplyAM(AMtype))
        //         currentState = st_AMheader;
        //     else
        //         currentState = st_sendReplyHeader;
        // }
        if (!(isAsyncAM(AMtype) || isReplyAM(AMtype))){
            axis_word.data = hdencode(axis_word.data, AM_SRC, AMdst);
            axis_word.data = hdencode(axis_word.data, AM_DST, AMsrc);
            #ifdef USE_ABS_PAYLOAD
            hdextract(axis_word.data, AM_PAYLOAD_SIZE) = GC_DATA_BYTES;
            #else
            axis_word.data = hdencode(axis_word.data, AM_PAYLOAD_SIZE, 0);
            #endif
            axis_word.data = hdencode(axis_word.data, AM_HANDLER, H_INCR_MEM);
            axis_word.data = hdencode(axis_word.data, AM_TYPE, AM_SHORT + AM_REPLY);
            axis_word.data = hdencode(axis_word.data, AM_HANDLER_ARGS, 0);
            axis_word.keep = GC_DATA_TKEEP;
            axis_word.dest = AMsrc;
            axis_word.last = 0;
            ATOMIC_ACTION(printWord("   Writing to loopback 5 - ", axis_word));
            out->write(axis_word);
            axis_word.data = 0;
            axis_word.data = hdencode(axis_word.data, AM_TOKEN, AMToken);
            // hdextract(axis_word.data, 39,) = 0;
            axis_word.last = 1; //!needs to be handled better
            axis_word.keep = GC_DATA_TKEEP;
            ATOMIC_ACTION(printWord("   Writing to loopback 6 - ", axis_word));
            out->write(axis_word);
        }
    }
}

void am_rx(galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out, word_t *arg, gc_AMtype_t* function,
    gc_AMargs_t* numargs, gc_AMhandler_t* handler, gc_AMToken_t* token,
    gc_payloadSize_t* payload){

    galapagos::stream_packet <word_t> axis_word;

    gc_AMsrc_t AMsrc;
    gc_AMdst_t AMdst;
    gc_AMToken_t AMToken;
    gc_AMtype_t AMtype;
    gc_AMargs_t AMargs;
    gc_AMhandler_t AMhandler;
    gc_AMdest_t AMdest;

    gc_payloadSize_t AMpayloadSize;
    gc_dstVectorNum_t AMdstVectorNum;

    gc_destination_t AMdestination;
    gc_strideBlockSize_t AMstrideBlockSize = 0;
    gc_strideBlockNum_t AMstrideBlockNum;
    gc_stride_t AMstride;

    gc_vectorSize_t AMvectorSize[MAX_VECTOR_NUM];
    gc_destination_t AMvectorDest[MAX_VECTOR_NUM];

    int i;

    axis_word = in->read();
    AMsrc = hdextract(axis_word.data, AM_SRC);
    AMdst = hdextract(axis_word.data, AM_DST);
    #ifdef USE_ABS_PAYLOAD
    AMpayloadSize = hdextract(axis_word.data, AM_PAYLOAD_SIZE) - GC_DATA_BYTES;
    #else
    AMpayloadSize = hdextract(axis_word.data, AM_PAYLOAD_SIZE);
    #endif
    AMhandler = hdextract(axis_word.data, AM_HANDLER);
    AMtype = hdextract(axis_word.data, AM_TYPE);
    AMargs = hdextract(axis_word.data, AM_HANDLER_ARGS);
    ATOMIC_ACTION(printWord("   Writing ", axis_word));
    out->write(axis_word);
    if (isReplyAM(AMtype) && (!isShortAM(AMtype))){
        while(axis_word.last != 1){
            axis_word = in->read();
            ATOMIC_ACTION(printWord("   Writing ", axis_word));
            out->write(axis_word);
        }
    } else {
        if(isLongStridedAM(AMtype)){
            axis_word = in->read();
            ATOMIC_ACTION(printWord("   Writing ", axis_word));
            out->write(axis_word);
            AMstride = hdextract(axis_word.data, AM_STRIDE_SIZE);
            AMstrideBlockSize = hdextract(axis_word.data, AM_STRIDE_BLK_SIZE);
            AMstrideBlockNum = hdextract(axis_word.data, AM_STRIDE_BLK_NUM);
            AMToken = hdextract(axis_word.data, AM_TOKEN);
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            #endif
            axis_word = in->read();
            AMdestination = axis_word.data;
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            gc_strideBlockSize_t payload = isLongAM(AMtype) ? (gc_strideBlockSize_t)(AMpayloadSize - AMargs*GC_DATA_BYTES) : (gc_strideBlockSize_t)(AMstrideBlockSize);
            #else
            gc_strideBlockSize_t payload = isLongAM(AMtype) ? AMpayloadSize : AMstrideBlockSize;
            #endif
            addr_word_t address = AMdestination & (power<2,GC_ADDR_WIDTH>()-1);
            // dataMoverWriteCommand(axis_s2mmCommand, 0, 0, address,
            //     address(1,0) != 0, 1, address(1,0), 1, payload);
        }
        else if(isLongVectoredAM(AMtype)){
            axis_word = in->read();
            AMdstVectorNum = hdextract(axis_word.data, AM_DST_VECTOR_NUM);
            AMvectorSize[0] = hdextract(axis_word.data, AM_DST_VECTOR_SIZE_HEAD);
            AMToken = hdextract(axis_word.data, AM_TOKEN);
            ATOMIC_ACTION(printWord("   Writing ", axis_word));
            out->write(axis_word);
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            #endif

            axis_word = in->read();
            AMvectorDest[0] = axis_word.data;
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            #endif
            // dataMoverWriteCommand(axis_s2mmCommand, 0, 0, 
            //     AMvectorDest[0](GC_ADDR_WIDTH-1,0),
            //     AMvectorDest[0](1,0) != 0, 1, 
            //     AMvectorDest[0](1,0), 1, AMvectorSize[0]);

            gc_dstVectorNum_t i = 0;
            for(i = 1; i < AMdstVectorNum; i++){
                axis_word = in->read();
                AMvectorSize[i] = hdextract(axis_word.data, AM_DST_VECTOR_SIZE_BODY);
                #ifdef USE_ABS_PAYLOAD
                AMpayloadSize-=GC_DATA_BYTES;
                #endif
                axis_word = in->read();
                AMvectorDest[i] = axis_word.data;
                #ifdef USE_ABS_PAYLOAD
                AMpayloadSize-=GC_DATA_BYTES;
                #endif
                // dataMoverWriteCommand(axis_s2mmCommand, 0, 0, 
                //     AMvectorDest[i](GC_ADDR_WIDTH-1,0),
                //     AMvectorDest[i](1,0) != 0, 1, 
                //     AMvectorDest[i](1,0), 1, AMvectorSize[i]);
            }
            
            // currentState = AMargs == 0 ? st_AMpayload : st_AMHandlerArgs;
        }
        else{ // st_AMToken
            axis_word = in->read();
            ATOMIC_ACTION(printWord("   Writing ", axis_word));
            out->write(axis_word);
            AMToken = hdextract(axis_word.data, AM_TOKEN);
            #ifdef USE_ABS_PAYLOAD
            AMpayloadSize-=GC_DATA_BYTES;
            #endif
            if(isLongAM(AMtype)){
                axis_word = in->read();
                AMdestination = axis_word.data;
                // ATOMIC_ACTION(printWord("   Writing ", axis_word));
                // out->write(axis_word);
                #ifdef USE_ABS_PAYLOAD
                AMpayloadSize-=GC_DATA_BYTES;
                gc_strideBlockSize_t payload = isLongAM(AMtype) ? (gc_strideBlockSize_t)(AMpayloadSize - AMargs*GC_DATA_BYTES) : (gc_strideBlockSize_t)(AMstrideBlockSize);
                #else
                gc_strideBlockSize_t payload = isLongAM(AMtype) ? AMpayloadSize : AMstrideBlockSize;
                #endif
                addr_word_t address = AMdestination & (power<2,GC_ADDR_WIDTH>()-1);
            }
        }

        if (AMargs != 0){
            for(int i = 0; i < AMargs; i++){
                axis_word = in->read();
                arg[i] = axis_word.data;
            }
        }

        gc_payloadSize_t i = 0;
        gc_payloadSize_t writeCount = 0;
        gc_destination_t strideDest = AMdestination + AMstride;
        gc_strideBlockNum_t strideCount = 1;
        gc_dstVectorNum_t vectorCount = 0;
        AMpayloadSize = AMpayloadSize % GC_DATA_BYTES == 0 ? AMpayloadSize / GC_DATA_BYTES : (gc_payloadSize_t)((AMpayloadSize / GC_DATA_BYTES) + 1);
        while(i < AMpayloadSize){
            axis_word = in->read();
            writeCount+=GC_DATA_BYTES;
            i++;
            if(isMediumAM(AMtype)){
                ATOMIC_ACTION(printWord("   Writing ", axis_word));
                out->write(axis_word);
            }
            else{
                if(isLongAM(AMtype)){
                    *(word_t*)(&(gasnet_shared_mem[AMdestination + writeCount - GC_DATA_BYTES])) = axis_word.data;
                    // axis_s2mm.write(axis_word);
                }
                else if(isLongStridedAM(AMtype)){
                    if (strideCount < AMstrideBlockNum){
                        strideDest += AMstride;
                        strideCount++;
                    }
                    // if(writeCount < AMstrideBlockSize){
                    //         // axis_word.last = 0;
                    //         // axis_s2mm.write(axis_word);
                    //         *(word_t*)(&(gasnet_shared_mem[strideDest + writeCount - GC_DATA_BYTES])) = axis_word.data;
                    // }
                    // else if(writeCount == AMstrideBlockSize){
                    //         axis_word.last = 1;
                    //         axis_s2mm.write(axis_word);
                    // }
                    if (writeCount <= AMstrideBlockSize){
                        *(word_t*)(&(gasnet_shared_mem[strideDest + writeCount - GC_DATA_BYTES])) = axis_word.data;
                    }
                    else{
                        ap_uint<GC_DATA_BYTES> keep = writeCount - AMstrideBlockSize;
                        memcpy(&(gasnet_shared_mem[strideDest + writeCount - GC_DATA_BYTES]), &(axis_word.data), keep);
                        // axis_word.last = 1;
                        // axis_word.keep = keep;
                        // axis_s2mm.write(axis_word);
                        writeCount = 0;
                    }
                }
                else{
                    if(writeCount <= AMvectorSize[vectorCount]){
                        // axis_word.last = 0;
                        // axis_s2mm.write(axis_word);
                        *(word_t*)(&(gasnet_shared_mem[AMvectorDest[vectorCount] + writeCount - GC_DATA_BYTES])) = axis_word.data;
                    }
                    // else if(writeCount == AMvectorSize[vectorCount]){
                    //     axis_word.last = 1;
                    //     axis_s2mm.write(axis_word);
                    // }
                    else{
                        vectorCount++;
                        // dataMoverWriteCommand(axis_s2mmCommand, 0, 0, 
                        //     AMvectorDest[vectorCount], //address
                        //     AMvectorDest[vectorCount](1,0) != 0, //ddr 
                        //     1, //eof
                        //     AMvectorDest[vectorCount](1,0), 1, //dsa, type
                        //     AMvectorSize[vectorCount]*GC_DATA_BYTES);
                        // if(!axis_s2mm.full()){
                            // axis_word.last = 0;
                            // axis_s2mm.write(axis_word);
                        // }
                        writeCount = 1;
                    }
                }
            }
        }
    }
    *function = AMtype;
    *numargs = AMargs;
    *handler = AMhandler;
    *token = AMToken;
}

void handler_thread(void (*fcnPtr)(galapagos::stream <word_t>* , 
    galapagos::stream <word_t>*), int id, galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out){
	
	unsigned int srcnode;
    gc_AMdst_t dstnode;
	gc_AMToken_t token;

	gc_AMtype_t function;
	gc_AMargs_t numargs;
	gc_AMhandler_t handler;
    gc_payloadSize_t payloadSize;

	word_t arg[power<2, AM_HANDLER_ARGS_WIDTH>()];
    word_t payload[16];
    int i;

    std::unique_ptr<galapagos::stream <word_t> > kernel_in;
    kernel_in = std::make_unique <galapagos::stream <word_t> > ();

    std::unique_ptr<galapagos::stream <word_t> > kernel_out;
    kernel_out = std::make_unique <galapagos::stream <word_t> > ();

    std::unique_ptr<galapagos::stream <word_t> > am_xpams_rx;
    am_xpams_rx = std::make_unique <galapagos::stream <word_t> > ();

    std::unique_ptr<galapagos::stream <word_t> > am_xpams_out;
    am_xpams_out = std::make_unique <galapagos::stream <word_t> > ();

    std::unique_ptr<galapagos::stream <word_t> > am_tx_in;
    am_tx_in = std::make_unique <galapagos::stream <word_t> > ();

    kernel_done[id] = new std::atomic_bool(false);
    std::atomic_bool* done = kernel_done[id];

    nodedata = &(gasnet_nodedata_all[id]);
    mutex_nodedata_global[id] = new mutex_t;
    mutex_nodedata = mutex_nodedata_global[id];

    thread_t kernel_thread = std::thread(fcnPtr, kernel_in.get(), kernel_out.get());

    SAFE_COUT("Handler " << id << " starting with nodedata at " << nodedata << "\n");

    while(1){

        while((in->empty() && kernel_out->empty() && am_xpams_out->empty()) && !(*done)){
            sched_yield();
        };
        if (*(done) && (in->empty() && kernel_out->empty() && am_xpams_out->empty())){
            SAFE_COUT("Joining thread in kernel " << id << "\n");
            kernel_thread.join();
            break;
        }

        galapagos::stream_packet <word_t> axis_word;
        if (gasnet_shared_mem == nullptr){
            gasnet_shared_mem = gasnet_shared_mem_global[id];
        }

        if (!in->empty()){
            SAFE_COUT("Data arrived in handler " << id << " from network\n");
            am_rx(in, am_xpams_rx.get(), arg, &function, &numargs, &handler, &token, &payloadSize);
            SAFE_COUT("   Data arrived in xpams " << id << " from am_rx\n");
            xpams_rx(am_xpams_rx.get(), am_xpams_out.get(), kernel_in.get());

            i = payloadSize;

        // axis_word = in->read();

        // // extract message parameters
        // // srcnode  = hdextract(packet_buffer_uint,nnSrcnode); //destnode = hdextract(packet_buffer_uint,nnDestnode);
        // // srcnode = (axis_word.data & AM_SRC_BITMASK) >> AM_SRC_LOWER;
        // srcnode = hdextract(axis_word.data, AM_SRC);
        // dstnode = hdextract(axis_word.data, AM_DST);
        // function = hdextract(axis_word.data, AM_TYPE);
        // numargs = hdextract(axis_word.data, AM_HANDLER_ARGS);
        // handler = hdextract(axis_word.data, AM_HANDLER);

        // // ! read token; should be conditional (kernel - kernel messages)
        // axis_word = in->read();

        // int token = (axis_word.data & AM_TOKEN_BITMASK) >> AM_TOKEN_LOWER;

        // if (numargs != 0){
        //     for(i = 0; i < numargs; i++){
        //         axis_word = in->read();
        //         arg[i] = axis_word.data;
        //     }
        // }

        // i = 0;
        // while(!axis_word.last){
        //     axis_word = in->read();
        //     payload[i] = axis_word.data;
        //     i++;
        // }
        
        // handler calls
        if (isShortAM(function))
            switch(numargs)
            {
                case 0:
                    ((void(*)(gc_AMToken_t
                                ))handlertable[handler])(token);break;
                case 1:
                    ((void(*)(gc_AMToken_t,garg
                                ))handlertable[handler])(token,arg[0]);break;
                case 2:
                    ((void(*)(gc_AMToken_t,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1]);break;
                case 3:
                    ((void(*)(gc_AMToken_t,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2]);break;
                case 4:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3]);break;
                case 5:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4]);break;
                case 6:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5]);break;
                case 7:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6]);break;
                case 8:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7]);break;
                case 9:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8]);break;
                case 10:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9]);break;
                case 11:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10]);break;
                case 12:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11]);break;
                case 13:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12]);break;
                case 14:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13]);break;
                case 15:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13],arg[14]);break;
                case 16:
                    ((void(*)(gc_AMToken_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13],arg[14],arg[15]);break;
                default:
                    break;
            }
        else
            //! payload isn't being passed in atm
            switch(numargs)
            {
                case 0:
                    ((void(*)(gc_AMToken_t,void*,size_t
                                ))handlertable[handler])(token,payload,i);break;
                case 1:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg
                                ))handlertable[handler])(token,payload,i,arg[0]);break;
                case 2:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1]);break;
                case 3:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2]);break;
                case 4:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3]);break;
                case 5:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4]);break;
                case 6:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5]);break;
                case 7:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6]);break;
                case 8:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7]);break;
                case 9:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8]);break;
                case 10:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9]);break;
                case 11:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10]);break;
                case 12:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11]);break;
                case 13:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12]);break;
                case 14:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13]);break;
                case 15:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13],arg[14]);break;
                case 16:
                    ((void(*)(gc_AMToken_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
                                ))handlertable[handler])(token,payload,i,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13],arg[14],arg[15]);break;
                default:
                    break;
            }

        // if(isMediumAM(function)){
        //     int j = 0;
        //     for(j = 0; j < i-1; j++){
        //         axis_word.data = payload[j];
        //         axis_word.last = 0;
        //         axis_word.keep = GC_DATA_TKEEP;
        //         kernel_in->write(axis_word);
        //     }
        //     axis_word.data = payload[j];
        //     axis_word.last = 1;
        //     axis_word.keep = GC_DATA_TKEEP;
        //     kernel_in->write(axis_word);
        // }

        // if(isReplyAM(function)){
        //     // ? need to add reply flag on AM type
        //     // sendShortAM(dstnode, srcnode, token, H_EMPTY, 0, nullptr, *out);
        // }

        }

        if(!kernel_out->empty()){
            SAFE_COUT("Data arrived in handler " << id << " from kernel\n");
            do{
                axis_word = kernel_out->read();
                am_tx_in->write(axis_word);
            } while(!axis_word.last);
            am_tx(am_tx_in.get(), out);
        } 
        
        if(!am_xpams_out->empty()){
            SAFE_COUT("Data arrived in handler " << id << " from loopback\n");
            do{
                axis_word = am_xpams_out->read();
                am_tx_in->write(axis_word);
            } while(!axis_word.last);
            am_tx(am_tx_in.get(), out);
        }
    };

    delete done;
    delete mutex_nodedata;
}
