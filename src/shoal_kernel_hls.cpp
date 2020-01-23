#include <iostream>

#ifndef __HLS__
#define __HLS__
#endif
#include "shoal_kernel.hpp"
#include "config.hpp"
#include "active_messages.hpp"
#include "handler_register_map.hpp"
#undef __HLS__

shoal::kernel::kernel(int id, int kernel_num, galapagos::interface<word_t> * in,
    galapagos::interface<word_t> * out, int * handler_ctrl)
{
    this->id = id;
    this->kernel_num = kernel_num;
    this->in = in;
    this->out = out;
    this->handler_ctrl = handler_ctrl;
}

int shoal::kernel::get_id(){
    return this->id;
}

int shoal::kernel::init(){
    // SAFE_COUT("*** Initializing kernel ***\n");
	// mutex_nodedata = mutex_nodedata_global[this->id];
    return 0;
}

// void InternalBarrierUpdate(){
//     lock_guard_t lck(*mutex_nodedata);
// 	SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Updating barrier_cnt at " << nodedata << " to " << nodedata->barrier_cnt+1 << "\n"));
//     nodedata->barrier_cnt++;
// 	return;
// }

// void MemReadyBarrierUpdate(){
//     lock_guard_t lck(*mutex_nodedata);
// 	SAFE_COUT(COLOR(Color::FG_BLUE, dec, "Updating mem_ready_barrier_cnt at " << nodedata << " to " << nodedata->mem_ready_barrier_cnt + 1 << "\n"));
//     nodedata->mem_ready_barrier_cnt++;
// 	return;
// }

// void counterUpdate(word_t arg){
//     lock_guard_t lck(*mutex_nodedata);
// 	SAFE_COUT("Updating counter at " << nodedata << " to " << nodedata->counter + arg << "\n");
//     nodedata->counter += arg;
// 	return;
// }

// void emptyHandler(){
// 	SAFE_COUT("Empty handler\n");
// 	return;
// }

void shoal::kernel::wait_mem(unsigned int value){
    unsigned int read_value;
    do{
        read_value = *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_MEM_READY_OUT_V_DATA);
    } while(read_value < value);
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_ARG_V_DATA) = value;
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_V_DATA) = H_INCR_MEM | 0x10;
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_V_DATA) = H_INCR_MEM;
}

void shoal::kernel::wait_barrier(unsigned int value){
    unsigned int read_value;
    do{
        read_value = *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_MEM_READY_OUT_V_DATA);
    } while(read_value < value);
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_ARG_V_DATA) = value;
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_V_DATA) = H_INCR_BAR | 0x10;
    *(this->handler_ctrl + XHANDLER_HANDLER_CTRL_BUS_ADDR_CONFIG_V_DATA) = H_INCR_BAR;
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
    sendShortAM_normal(dst, 0, H_INCR_BAR, 0, nullptr);
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
    // SAFE_COUT("Leaving kernel " << this->id << "\n");

    // free(gasnet_shared_mem);
    // *kernel_done[this->id] = true;
    // should free handlertable but it may segfault in the handler
}
