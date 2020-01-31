#include <iostream>
#include "shoal_kernel.hpp"
#include "global_utilities.hpp"
#include "am_globals.hpp"
#include "config.hpp"
#include "active_messages.hpp"

shoal::kernel::kernel(int id, int kernel_num, galapagos::interface <word_t> * in,
    galapagos::interface <word_t> * out)
{
    this->id = id;
    this->kernel_num = kernel_num;
    this->in = in;
    this->out = out;
}

int shoal::kernel::get_id(){
    return this->id;
}

int shoal::kernel::init(){
    // std::cout << "*** Initializing kernel ***\n";
	mutex_nodedata = mutex_nodedata_global[this->id];
    return 0;
}

void InternalBarrierUpdate(){
    lock_guard_t lck(*mutex_nodedata);
	SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Updating barrier_cnt at " << nodedata << " to " << nodedata->barrier_cnt+1 << "\n"));
    nodedata->barrier_cnt++;
	return;
}

void MemReadyBarrierUpdate(){
    lock_guard_t lck(*mutex_nodedata);
	SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Updating mem_ready_barrier_cnt at " << nodedata << " to " << nodedata->mem_ready_barrier_cnt + 1 << "\n"));
    nodedata->mem_ready_barrier_cnt++;
	return;
}

void counterUpdate(word_t arg){
    lock_guard_t lck(*mutex_nodedata);
	SAFE_COUT("Updating counter at " << nodedata << " to " << nodedata->counter + arg << "\n");
    nodedata->counter += arg;
	return;
}

void emptyHandler(){
	SAFE_COUT("Empty handler\n");
	return;
}

int shoal::kernel::attach(gasnet_handlerentry_t *table, int numentries, int size){
    // build handler table
	allocate_handlerTable();

	// fill in application handlers
	for(int t = 0; t < numentries; t++)
		handlertable[table[t].index] = (void*) table[t].fnptr;

	// fill in private GASNet handlers
    handlertable[H_EMPTY] = (void*)emptyHandler;
	handlertable[H_INCR_BAR] = (void*)InternalBarrierUpdate;
	handlertable[H_INCR_MEM] = (void*)MemReadyBarrierUpdate;
	handlertable[H_ADD] = (void*)counterUpdate;

    gasnet_shared_mem = (std::byte*) calloc(size,sizeof(std::byte));
	if (!gasnet_shared_mem){
		fprintf(stderr,"Shared segment could not be allocated.\r\n");
		abort();
    }
    gasnet_shared_mem_global[this->id] = gasnet_shared_mem;

    nodedata = &(gasnet_nodedata_all[this->id]);

    // ? no long messages should be sent until all kernels are here

    return 0;
}

void shoal::kernel::allocate_handlerTable(){
	lock_guard_t guard(mutex_nodeInit);
	if (!handlertable){
		handlertable = (void **)calloc((size_t)(power<2, AM_HANDLER_ARGS_WIDTH>()),sizeof(void*));
		if (!handlertable){
			fprintf(stderr,"Handlertable could not be allocated.\r\n");
			exit(1);
		}
	}
}

void shoal::kernel::wait_mem(unsigned int value){
    while(nodedata->mem_ready_barrier_cnt < value){
        sched_yield();
    };
    lock_guard_t lck(*mutex_nodedata);
    SAFE_COUT("PROFILE:S:interrupt_V:1\n")
    SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Resetting mem_ready_barrier_cnt at " << nodedata << " to " << nodedata->mem_ready_barrier_cnt - 1 << "\n"));
    nodedata->mem_ready_barrier_cnt -= value;
    SAFE_COUT("PROFILE:S:interrupt_V:0\n")
}

void shoal::kernel::wait_barrier(unsigned int value){
    while(nodedata->barrier_cnt < value){
        sched_yield();
    };
    lock_guard_t lck(*mutex_nodedata);
    SAFE_COUT("PROFILE:S:interrupt_V:1\n")
    SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Resetting barrier at " << nodedata << " to " << nodedata->barrier_cnt - 1 << "\n"));
    nodedata->barrier_cnt -= value;
    SAFE_COUT("PROFILE:S:interrupt_V:0\n")
}

void shoal::kernel::sendShortAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, word_t * handler_args)
{
    sendShortAM(AM_SHORT, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, *(this->out));
}

void shoal::kernel::sendShortAM_async(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, word_t * handler_args)
{
    sendShortAM(AM_SHORT + AM_ASYNC, this->id, dst, token, handlerID,
        handlerArgCount, handler_args, *(this->out));
}

void shoal::kernel::sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t* payload)
{
    sendMediumAM(AM_MEDIUM|AM_FIFO, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, payload, *(this->out));
}

void shoal::kernel::sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t src_addr)
{
    sendMediumAM(AM_MEDIUM, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, src_addr, *(this->out));
}

void shoal::kernel::sendMediumAM_async(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t* payload)
{
    sendMediumAM(AM_MEDIUM|AM_FIFO|AM_ASYNC, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, payload, *(this->out));
}

void shoal::kernel::sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t* payload, word_t dst_addr)
{
    sendLongAM(AM_LONG|AM_FIFO, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, payload, dst_addr, *(this->out));
}

void shoal::kernel::sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t src_addr, word_t dst_addr)
{
    sendLongAM(AM_LONG, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, src_addr, dst_addr, *(this->out));
}

void shoal::kernel::sendMemUpdate(gc_AMdst_t dst){
    sendShortAM_normal(dst, 0, H_INCR_MEM, 0, nullptr);
}

void shoal::kernel::sendBarrierUpdate(gc_AMdst_t dst){
    sendShortAM_normal(dst, 0xABCD, H_INCR_BAR, 0, nullptr);
}

void shoal::kernel::barrier_wait(){
    this->wait_barrier(this->kernel_num-1);
    for(int i = 0; i < this->kernel_num; i++){
        if (i != this->id){
            ATOMIC_ACTION(this->sendBarrierUpdate(i));
        }
    }
    this->wait_reply(kernel_num-1);
}

void shoal::kernel::barrier_send(int id){
    ATOMIC_ACTION(this->sendBarrierUpdate(id));
    this->wait_barrier(1);
    this->wait_reply(1);
}

void shoal::kernel::wait_reply(unsigned int value){
    this->wait_mem(value);
}

void shoal::kernel::end(){
    SAFE_COUT("Leaving kernel " << this->id << "\n");

    free(gasnet_shared_mem);
    *kernel_done[this->id] = true;
    // should free handlertable but it may segfault in the handler
    // free(handlertable);
}
