#ifndef __HLS__
#define __HLS__
#endif
#include <cstddef> // needed to resolve ::max_align_t errors
#include <cstring>
#include "hls_kernel.hpp"

// this function, if inlined, or placed in code, behaves badly in Verilog. In 
// simulation, it does do three writes but they appear as burst writes to the wrong
// addresses. Doing it this way seems to work properly. Tested in HLS 2018.1
void start_timer_func(volatile int* axi_timer){
    axi_timer[1] = 0; // set load register to 0
    axi_timer[0] = 0x10; // load timer with load register
    axi_timer[0] = 0x40; // start timer
}

extern "C"{
void hls_kernel(
    short id,
    galapagos::interface <word_t> * in,
    galapagos::interface<word_t> * out,
    volatile int * handler_ctrl,
    volatile int * axi_timer,
    word_t * instr_mem,
    word_t * local_mem
){
    #pragma HLS INTERFACE axis port=in
    #pragma HLS INTERFACE axis port=out
    #pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE m_axi port=handler_ctrl depth=4096 offset=0
    #pragma HLS INTERFACE m_axi port=axi_timer depth=4096 offset=0
    #pragma HLS INTERFACE m_axi port=instr_mem depth=4096 offset=0
    #pragma HLS INTERFACE m_axi port=local_mem depth=4096 offset=0


    shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out, handler_ctrl);
    galapagos::stream_packet <word_t> axis_word;
    static int pc;

    static gc_AMdst_t AMdst;
    static gc_AMToken_t AMtoken;
    static gc_AMhandler_t AMhandler;
    static gc_AMargs_t AMargs;
    static word_t handler_args [16];
    static gc_payloadSize_t payloadSize;
    static gc_AMsrc_t src_addr;
    static gc_AMdest_t dst_addr;
    int i;
    static short addr;

    instruction_t instruction = (instruction_t) *(instr_mem + pc++);
    // pc++;
    switch (instruction){
    case load:{
        AMdst = (instruction_t) *(instr_mem + (pc++));
        AMtoken = (instruction_t) *(instr_mem + (pc++));
        AMhandler = (instruction_t) *(instr_mem + (pc++));
        AMargs = (instruction_t) *(instr_mem + (pc++));
        for(i = 0; i < AMargs; i++){
            handler_args[i] = (instruction_t) *(instr_mem + (pc++));
        }
        payloadSize = (instruction_t) *(instr_mem + (pc++));
        src_addr = (instruction_t) *(instr_mem + (pc++));
        dst_addr = (instruction_t) *(instr_mem + (pc++));
        // pc++;
        break;
    }
    case send_short:{
        kernel.sendShortAM_normal(AMdst, AMtoken, AMhandler, AMargs, handler_args);
        AMtoken++;
        kernel.wait_reply(1);
        break;
    }
    case send_medium:{
        kernel.sendMediumAM_normal(AMdst, AMtoken, AMhandler, AMargs, handler_args, payloadSize, src_addr);
        AMtoken++;
        kernel.wait_reply(1);
        break;
    }
    case send_medium_fifo:{
        int i = 0;
        word_t payload[16];
        for(i = 0; i < payloadSize/GC_DATA_BYTES; i++){
            payload[i] = (instruction_t) *(instr_mem + (pc++));
        }
        kernel.sendMediumAM_normal(AMdst, AMtoken, AMhandler, AMargs, handler_args, payloadSize, payload);
        kernel.wait_reply(1);
        break;
    }
    case send_long:{
        kernel.sendLongAM_normal(AMdst, AMtoken, AMhandler, AMargs, handler_args, payloadSize, src_addr, dst_addr);
        AMtoken++;
        kernel.wait_reply(1);
        break;
    }
    case send_long_fifo:{
        int i = 0;
        word_t payload[16];
        for(i = 0; i < payloadSize/GC_DATA_BYTES; i++){
            payload[i] = (instruction_t) *(instr_mem + (pc++));
        }
        kernel.sendLongAM_normal(AMdst, AMtoken, AMhandler, AMargs, handler_args, payloadSize, payload, dst_addr);
        break;
    }
    case start_timer:{
        // int i = 0;
        // int j = 0x10;
        // int k = 0x40;
        // axi_timer[1] = 0; // set load register to 0
        // axi_timer[0] = 0x10; // load timer with load register
        // axi_timer[0] = 0x40; // start timer
        // std::memset((void *)(axi_timer + 1), i, 4);
        // std::memset((void *)(axi_timer + 0), j, 4);
        // std::memset((void *)(axi_timer + 0), k, 4);
        start_timer_func(axi_timer);
        break;
    }
    case stop_timer:{
        axi_timer[0] = 0x0; // stop timer
        word_t time = *(axi_timer + 0x2); // read timer count
        // gc_AMdst_t AMdst_tmp = (instruction_t) *(instr_mem + (pc++));
        kernel.sendMediumAM_async(0, AMtoken, H_EMPTY, 0, handler_args, 8, &time);
        AMtoken++;
        break;
    }
    case barrier_send:{
        word_t dst = (instruction_t) *(instr_mem + (pc++));
        kernel.barrier_send(dst);
        break;
    }
    case barrier_wait:{
        kernel.barrier_wait();
        break;
    }
    case recv_medium:{
        axis_word = in->read();
        break;
    }
    case read_local:{
        word_t addr = (word_t) *(instr_mem + (pc++));
        word_t read_value = *(local_mem + addr);
        *(local_mem + addr + 1) = read_value;
        break;
    }
    case end:{
        while(1){}
        break;
    }
    default:
        break;
    }
}
}
