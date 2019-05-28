#include "am_globals.hpp"
#include "am_gasnet.hpp"
#include "global_utilities.hpp"
#include "config.hpp"

#define CPU
#include "active_messages.hpp"

void allocate_handlerTable(){
	lock_guard_t guard(mutex_nodeInit);
	if (!handlertable){
		handlertable = (void **)calloc((size_t)(power<2, AM_HANDLER_ARGS_WIDTH>()),sizeof(void*));
		if (!handlertable){
			fprintf(stderr,"Handlertable could not be allocated.\r\n");
			exit(1);
		}
	}
}

void InternalBarrierUpdate(){
	nodedata->barrier_cnt++;
	return;
}

void MemReadyBarrierUpdate(){
	nodedata->mem_ready_barrier_cnt++;
	return;
}

void counterUpdate(int arg){
	nodedata->counter+=arg;
	return;
}

void emptyHandler(){
	std::cout << "Empty handler\n";
	return;
}

int gasnet_attach(gasnet_handlerentry_t *table, int numentries){
	// build handler table
	allocate_handlerTable();
	
	// fill in application handlers
	for(int t = 0; t < numentries; t++)
		handlertable[table[t].index] = (void*) table[t].fnptr;

	// fill in private GASNet handlers
    handlertable[0] = (void*)emptyHandler;
	handlertable[H_INCR_BAR] = (void*)InternalBarrierUpdate;
	handlertable[H_INCR_MEM] = (void*)MemReadyBarrierUpdate;
	handlertable[H_ADD] = (void*)counterUpdate;
	
	lock_t lock(mutex_nodeInit, std::defer_lock);
	lock.lock();
	
	gasnet_shared_mem = new void *[SHARED_VARIABLE_NUM];
	if (!gasnet_shared_mem){
		fprintf(stderr,"Shared segment could not be allocated.\r\n");
		abort();
	}
	
	return 0;
}

void handler_thread(void (*fcnPtr)(galapagos::stream <word_t>* , 
    galapagos::stream <word_t>*), int id, galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out){
	
	unsigned int srcnode;
	gc_AMToken_t token;

	unsigned int function;
	unsigned int numargs;
	gc_AMhandler_t handler;

	gc_AMargs_t arg[power<2, AM_HANDLER_ARGS_WIDTH>()];
    word_t payload[16];
    int i;

    std::unique_ptr<galapagos::stream <word_t> > kernel_in;
    kernel_in = std::make_unique <galapagos::stream <word_t> > ();



    kernel_done[id] = new std::atomic_bool(false);
    std::atomic_bool* done = kernel_done[id];

    thread_t kernel_thread = std::thread(fcnPtr, kernel_in.get(), out);

    std::cout << "Handler " << id << " starting\n";

    do{

        while( in->empty() && !(*done) ){
            sched_yield();
        };
        if (*(done)){
            std::cout << "Joining thread in kernel " << id << "\n";
            kernel_thread.join();
            break;
        }
        std::cout << "Data in handler arrived\n";

        galapagos::stream_packet <word_t> axis_word;

        axis_word = in->read();

        // extract message parameters
        // srcnode  = hdextract(packet_buffer_uint,nnSrcnode); //destnode = hdextract(packet_buffer_uint,nnDestnode);
        srcnode = (axis_word.data & AM_SRC_BITMASK) >> AM_SRC_LOWER;
        function = (axis_word.data & AM_TYPE_BITMASK) >> AM_TYPE_LOWER;
        numargs = (axis_word.data & AM_HANDLER_ARGS_BITMASK) >> AM_HANDLER_ARGS_LOWER;
        handler = (axis_word.data & AM_HANDLER_BITMASK) >> AM_HANDLER_LOWER;

        // ! read token; should be conditional (kernel - kernel messages)
        axis_word = in->read();

        int token = (axis_word.data & AM_TOKEN_BITMASK) >> AM_TOKEN_LOWER;

        if (numargs != 0){
            for(i = 0; i < numargs; i++){
                axis_word = in->read();
                arg[i] = axis_word.data;
            }
        }

        i = 0;
        while(!axis_word.last){
            axis_word = in->read();
            payload[i] = axis_word.data;
            i++;
        }
        
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

        if(isMediumAM(function)){
            int j = 0;
            for(j = 0; j < i-1; j++){
                axis_word.data = payload[j];
                axis_word.last = 0;
                axis_word.keep = GC_DATA_TKEEP;
                kernel_in->write(axis_word);
            }
            axis_word.data = payload[j];
            axis_word.last = 1;
            axis_word.keep = GC_DATA_TKEEP;
            kernel_in->write(axis_word);
        }
    } while(!(*done));

    delete gasnet_shared_mem;
    delete done;
}
