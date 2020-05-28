#include <iostream>
#include <sstream>
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

    if(this->id == 1){
        for(long long i = 0; i < 512; i++){
            memcpy(&gasnet_shared_mem[i*8], &i, 8);
        }
        std::stringstream ss;
        ss << "Memory in node 1: ";
        for(int i = 0; i < 10; i++){
            ss << *((long long *)(&gasnet_shared_mem[i*8])) << ", ";
        }
        // ss << "...";
        // for(int i = 508; i < 512; i++){
        //     ss << *((long long *)(&gasnet_shared_mem[i*8])) << ", ";
        // }
        ss << "\n";
        std::cout << ss.str();
    }

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
    // SAFE_COUT("PROFILE:S:interrupt_V:1\n")
    SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Resetting mem_ready_barrier_cnt at " << nodedata << " to " << nodedata->mem_ready_barrier_cnt - value << "\n"));
    nodedata->mem_ready_barrier_cnt -= value;
    // SAFE_COUT("PROFILE:S:interrupt_V:0\n")
}

void shoal::kernel::wait_barrier(unsigned int value){
    while(nodedata->barrier_cnt < value){
        sched_yield();
    };
    lock_guard_t lck(*mutex_nodedata);
    // SAFE_COUT("PROFILE:S:interrupt_V:1\n")
    SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Resetting barrier at " << nodedata << " from " << nodedata->barrier_cnt << " to " << nodedata->barrier_cnt - value << "\n"));
    nodedata->barrier_cnt -= value;
    // SAFE_COUT("PROFILE:S:interrupt_V:0\n")
}

void shoal::kernel::wait_counter(unsigned int value){
    while(nodedata->counter < value){
        sched_yield();
    };
    lock_guard_t lck(*mutex_nodedata);
    // SAFE_COUT("PROFILE:S:interrupt_V:1\n")
    SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Resetting counter at " << nodedata << " from " << nodedata->counter << " to " << nodedata->counter - value << "\n"));
    nodedata->counter -= value;
    // SAFE_COUT("PROFILE:S:interrupt_V:0\n")
}

void shoal::kernel::sendShortAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args)
{
    sendShortAM(AM_SHORT, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, *(this->out));
}

void shoal::kernel::sendShortAM_async(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args)
{
    sendShortAM(AM_SHORT + AM_ASYNC, this->id, dst, token, handlerID,
        handlerArgCount, handler_args, *(this->out));
}

void shoal::kernel::sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize)
{
    sendMediumAM(AM_MEDIUM|AM_FIFO, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, *(this->out));
}

void shoal::kernel::sendPayload(gc_AMdst_t dst, word_t payload, bool assertLast){
    galapagos::stream_packet <word_t> axis_word;
    axis_word.dest = dst;
    axis_word.data = payload;
    axis_word.last = assertLast;
    axis_word.keep = GC_DATA_TKEEP;
    axis_word.id = 0;
    this->out->write(axis_word);
}

void shoal::kernel::sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t src_addr)
{
    sendMediumAM(AM_MEDIUM, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, src_addr, *(this->out));
}

void shoal::kernel::sendMediumAM_async(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize)
{
    sendMediumAM(AM_MEDIUM|AM_FIFO|AM_ASYNC, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, *(this->out));
}

void shoal::kernel::sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t dst_addr)
{
    sendLongAM(AM_LONG|AM_FIFO, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, dst_addr, *(this->out));
}

void shoal::kernel::sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize, word_t src_addr, word_t dst_addr)
{
    sendLongAM(AM_LONG, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, src_addr, dst_addr, *(this->out));
}

void shoal::kernel::sendLongStrideAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize, gc_stride_t src_stride, gc_strideBlockSize_t src_blk_size,
    gc_strideBlockNum_t src_blk_num, word_t src_addr, gc_stride_t dst_stride,
    gc_strideBlockSize_t dst_blk_size, gc_strideBlockNum_t dst_blk_num, word_t dst_addr)
{
    longStridedAM(AM_STRIDE, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, src_stride, src_blk_size, src_blk_num, src_addr, 
        dst_stride, dst_blk_size, dst_blk_num, dst_addr, *(this->out));
}

void shoal::kernel::sendLongVectorAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
    gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
    gc_payloadSize_t payloadSize, gc_srcVectorNum_t srcVectorCount, gc_dstVectorNum_t dstVectorCount,
    const gc_vectorSize_t * srcSize, const gc_vectorSize_t * dstSize, const word_t * src_addr,
    const word_t * dst_addr)
{
    longVectorAM(AM_VECTOR, this->id, dst, token, handlerID, handlerArgCount,
        handler_args, payloadSize, srcVectorCount, dstVectorCount, srcSize, dstSize, 
        src_addr, dst_addr, *(this->out));
}

void shoal::kernel::sendMemUpdate(gc_AMdst_t dst){
    sendShortAM_normal(dst, 0xcd, H_INCR_MEM, 0, nullptr);
}

void shoal::kernel::sendBarrierUpdate(gc_AMdst_t dst){
    sendShortAM_normal(dst, 0xce, H_INCR_BAR, 0, nullptr);
}

void shoal::kernel::barrier_wait(){
    this->wait_barrier(this->kernel_num-1);
    for(int i = 0; i < this->kernel_num; i++){
        if (i != this->id){
            ATOMIC_ACTION(this->sendBarrierUpdate(i));
        }
    }
    this->wait_reply(this->kernel_num-1);
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
    std::stringstream ss;
    if (this->id == 2){
        ss << "Memory in node 2 at end: ";
        for(int i = 0; i < 10; i++){
            ss << *((long long *)(&gasnet_shared_mem[i*8])) << ", ";
        }
        // ss << "...";
        // for(int i = 508; i < 512; i++){
        //     ss << *((long long *)(&gasnet_shared_mem[i*8])) << ", ";
        // }
        ss << "\n";
        std::cout << ss.str();
    }

    SAFE_COUT("Leaving kernel " << this->id << "\n");

    free(gasnet_shared_mem);
    *kernel_done[this->id] = true;
    // should free handlertable but it may segfault in the handler
    // free(handlertable);
}
