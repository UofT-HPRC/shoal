// #ifndef __HLS__
// #define __HLS__
// #endif
#include <cstddef> // needed to resolve ::max_align_t errors
#include <cstring>
#include "benchmark.hpp"
#include "ap_utils.h"

#ifndef __HLS__
#include <iostream>
#include <fstream> // To use ifstream
#endif

enum instruction_t{
    load, short_latency, medium_latency, long_latency, barrier_send,
    recv_medium, end, barrier_wait, long_fifo_latency, medium_fifo_latency,
    read_local, recv_time, add_label, short_throughput, medium_throughput, 
    long_throughput, medium_fifo_throughput, long_fifo_throughput, strided_latency,
    strided_throughput, vector_latency, vector_throughput, load_stride, load_vector,
    wait_counter, busy_loop, send_pilot, recv_pilot}
    current_instruction;

#ifndef __HLS__
auto start_timer(){
    return std::chrono::high_resolution_clock::now();
}

void stop_timer(shoal::kernel* kernel, gc_AMToken_t token, std::chrono::high_resolution_clock::time_point timer){
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - timer;
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    word_t time = (word_t)(elapsed_ns / 6.4); // convert to 156.25 MHz (6.4ns) cycles
    kernel->sendMediumAM_normal(0, token, H_EMPTY, 0, NULL, 8);
    kernel->sendPayload(0, time, true); // assume always send to kernel 0
    kernel->wait_reply(1);
}
#else
// this function, if inlined, or placed in code, behaves badly in Verilog. In
// simulation, it does do three writes but they appear as burst writes to the wrong
// addresses. Doing it this way seems to work properly. Tested in HLS 2018.1
void start_timer(volatile int* axi_timer){
    axi_timer[0] = 0x0; // stop timer
    axi_timer[1] = 0; // set load register to 0
    axi_timer[0] = 0x20; // load timer with load register
    axi_timer[0] = 0x80; // start timer
}

void stop_timer(shoal::kernel* kernel, gc_AMToken_t token, volatile int* axi_timer){
    #pragma HLS INLINE
    axi_timer[0] = 0x0; // stop timer
    word_t time = *(axi_timer + 0x2); // read timer count
    word_t dumb = *(axi_timer);
    if(dumb != 0xFF){
    kernel->sendMediumAM_normal(0, token, H_EMPTY, 0, NULL, 8);
    kernel->sendPayload(0, time, true); // assume always send to kernel 0
    }
    // axi_timer[0] = 0x0; // stop timer
    kernel->wait_reply(1, axi_timer);
}
#endif

#ifndef __HLS__
void print_time(std::chrono::high_resolution_clock::time_point timer, std::string label){
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - timer;
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    std::cout << label << ":" << elapsed_ns << std::endl;
}
#endif

extern "C"{
void benchmark(
    short id,
    galapagos::interface <word_t> * in,
    galapagos::interface<word_t> * out,
    #ifdef __HLS__
    volatile int * handler_ctrl,
    volatile int * axi_timer,
    #else
    int * instr_mem
    #endif
    #ifdef __HLS__
    int * instr_mem,
    word_t * local_mem
    #endif
){
    #pragma HLS INTERFACE axis port=in
    #pragma HLS INTERFACE axis port=out
    #pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_stable port=id
    #pragma HLS INTERFACE m_axi port=handler_ctrl depth=4096 offset=0
    #pragma HLS INTERFACE m_axi port=axi_timer depth=4096 offset=0
    #pragma HLS INTERFACE m_axi port=instr_mem depth=4096 offset=0
    #pragma HLS INTERFACE m_axi port=local_mem depth=4096 offset=0


    #ifdef __HLS__
    shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out, handler_ctrl);
    #else
    shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out);
    #endif

    kernel.init();
    #ifndef __HLS__
    kernel.attach(nullptr, 0, SEGMENT_SIZE);
    #endif
    galapagos::stream_packet <word_t> axis_word;
    int pc = 0;

    gc_AMhandler_t AMhandler = H_EMPTY;
    gc_AMargs_t AMargs = 0;
    word_t handler_args [16];
    gc_payloadSize_t payloadSize = 0;
    gc_AMsrc_t src_addr = 0;
    gc_AMdest_t dst_addr = 0;
    int i;
    short addr;

    instruction_t instruction;
    bool loop = true;

    const int AMdst = 2;
    word_t loopCount = 1;

    gc_stride_t src_stride = -1;
    gc_strideBlockSize_t src_blk_size = -1;
    gc_strideBlockNum_t src_blk_num = -1;
    gc_stride_t dst_stride = -1;
    gc_strideBlockSize_t dst_blk_size = -1;
    gc_strideBlockNum_t dst_blk_num = -1;

    gc_srcVectorNum_t srcVectorCount = -1;
    gc_dstVectorNum_t dstVectorCount = -1;
    gc_vectorSize_t srcSize[16];
    word_t src_addrs[16];
    gc_vectorSize_t dstSize[16];
    word_t dst_addrs[16];

    while(loop){
    instruction = (instruction_t) *(instr_mem + pc++);
    // pc++;
    switch (instruction){
    case load:{
        AMhandler = (gc_AMhandler_t) *(instr_mem + (pc++));
        AMargs = (gc_AMargs_t) *(instr_mem + (pc++));
        for(i = 0; i < AMargs; i++){
            handler_args[i] = (word_t) *(instr_mem + (pc++));
        }
        payloadSize = (gc_payloadSize_t) *(instr_mem + (pc++));
        src_addr = (gc_AMsrc_t) *(instr_mem + (pc++));
        dst_addr = (gc_AMdst_t) *(instr_mem + (pc++));
        // pc++;
        break;
    }
    case load_stride:{
        src_stride = (gc_stride_t) *(instr_mem + (pc++));
        src_blk_size = (gc_strideBlockSize_t) *(instr_mem + (pc++));
        src_blk_num = (gc_strideBlockNum_t) *(instr_mem + (pc++));
        dst_stride = (gc_stride_t) *(instr_mem + (pc++));
        dst_blk_size = (gc_strideBlockSize_t) *(instr_mem + (pc++));
        dst_blk_num = (gc_strideBlockNum_t) *(instr_mem + (pc++));
        break;
    }
    case load_vector:{
        srcVectorCount = (gc_srcVectorNum_t) *(instr_mem + (pc++));
        dstVectorCount = (gc_dstVectorNum_t) *(instr_mem + (pc++));
        for(i = 0; i < srcVectorCount; i++){
            srcSize[i] = (gc_vectorSize_t) *(instr_mem + (pc++));
        }
        for(i = 0; i < srcVectorCount; i++){
            src_addrs[i] = (word_t) *(instr_mem + (pc++));
        }
        for(i = 0; i < dstVectorCount; i++){
            dstSize[i] = (gc_vectorSize_t) *(instr_mem + (pc++));
        }
        for(i = 0; i < dstVectorCount; i++){
            dst_addrs[i] = (word_t) *(instr_mem + (pc++));
        }
        break;
    }
    case short_latency:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        for(i = 0; i < loopCount; i++){
            #ifndef __HLS__
            auto timer = start_timer();
            #else
            start_timer(axi_timer);
            #endif
            kernel.sendShortAM_normal(AMdst, 0xff0, AMhandler, AMargs, handler_args);
            // {
            //     #pragma HLS INLINE REGION
            //     kernel.wait_reply(1);
            // }
            // kernel.wait_reply(1);
            #ifndef __HLS__
            kernel.wait_reply(1);
            stop_timer(&kernel, 0xab0, timer);
            #else
            kernel.wait_reply(1, axi_timer);
            stop_timer(&kernel, 0xab0, axi_timer);
            #endif
        }
        break;
    }
    case medium_latency:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        for(i = 0; i < loopCount; i++){
            #ifndef __HLS__
            auto timer = start_timer();
            #else
            start_timer(axi_timer);
            #endif
            kernel.sendMediumAM_normal(AMdst, 0xff1, AMhandler, AMargs, handler_args, payloadSize, src_addr);
            // kernel.wait_reply(1);
            #ifndef __HLS__
            kernel.wait_reply(1);
            stop_timer(&kernel, 0xab1, timer);
            #else
            kernel.wait_reply(1, axi_timer);
            stop_timer(&kernel, 0xab1, axi_timer);
            #endif
        }
        break;
    }
    case medium_fifo_latency:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        word_t j = 0;
        for(i = 0; i < loopCount; i++){
            #ifndef __HLS__
            auto timer = start_timer();
            #else
            start_timer(axi_timer);
            #endif
            kernel.sendMediumAM_normal(AMdst, 0xff2, AMhandler, AMargs, handler_args, payloadSize);
            for (j = 0; j < payloadSize; j+=GC_DATA_BYTES){
                kernel.sendPayload(AMdst, j, j == payloadSize - ((gc_payloadSize_t)GC_DATA_BYTES));
            }
            // kernel.wait_reply(1);
            #ifndef __HLS__
            kernel.wait_reply(1);
            stop_timer(&kernel, 0xab2, timer);
            #else
            kernel.wait_reply(1, axi_timer);
            stop_timer(&kernel, 0xab2, axi_timer);
            #endif
        }
        break;
    }
    case long_latency:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        for(i = 0; i < loopCount; i++){
            #ifndef __HLS__
            auto timer = start_timer();
            #else
            start_timer(axi_timer);
            #endif
            kernel.sendLongAM_normal(AMdst, 0xff3, AMhandler, AMargs, handler_args, payloadSize, src_addr, dst_addr);
            // auto tmp2 = std::chrono::high_resolution_clock::now();
            // std::chrono::duration<double> elapsed = tmp2 - timer;
            // auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
            // word_t time = (word_t)(elapsed_ns / 6.4); // convert to 156.25 MHz (6.4ns) cycles
            // std::cout << "loop1: " << time << std::endl;
            // kernel.wait_reply(1);
            #ifndef __HLS__
            kernel.wait_reply(1);
            stop_timer(&kernel, 0xab3, timer);
            #else
            kernel.wait_reply(1, axi_timer);
            stop_timer(&kernel, 0xab3, axi_timer);
            #endif
        }
        break;
    }
    case long_fifo_latency:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        word_t j = 0;
        for(i = 0; i < loopCount; i++){
            #ifndef __HLS__
            auto timer = start_timer();
            #else
            start_timer(axi_timer);
            #endif
            kernel.sendLongAM_normal(AMdst, 0xff4, AMhandler, AMargs, handler_args, payloadSize, dst_addr);
            for (j = 0; j < payloadSize; j+=GC_DATA_BYTES){
                kernel.sendPayload(AMdst, j, j == payloadSize - ((gc_payloadSize_t)GC_DATA_BYTES));
            }
            // {
            //     doesn't work: the write happens all at one address instead of three
            //     #pragma HLS INLINE REGION
            //     kernel.wait_reply(1);
            // }
            // kernel.wait_reply(1);
            #ifndef __HLS__
            kernel.wait_reply(1);
            stop_timer(&kernel, 0xab4, timer);
            #else
            kernel.wait_reply(1, axi_timer);
            stop_timer(&kernel, 0xab4, axi_timer);
            #endif
        }
        break;
    }
    case barrier_send:{
        kernel.barrier_send(0); // assume always sending to kernel 0
        // std::cout << "Barrier sent from kernel " << kernel.get_id() << "\n";
        break;
    }
    case barrier_wait:{
        kernel.barrier_wait();
        // std::cout << "Barrier wait from kernel " << kernel.get_id() << "\n";
        break;
    }
    case recv_medium:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        int j = 0;
        for(i = 0; i < loopCount; i++){
            axis_word = in->read(); // read token
            for (j = 0; j < payloadSize; j+=GC_DATA_BYTES){
                axis_word = in->read();
            }
        }
        break;
    }
    case recv_time:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        for(i = 0; i < loopCount; i++){
            axis_word = in->read();
            axis_word = in->read();
            #ifndef __HLS__
            std::cout << "timing: " << axis_word.data << "\n";
            #endif
        }
        break;
    }
    case read_local:{
        #ifdef __HLS__
        word_t addr = (word_t) *(instr_mem + (pc++));
        word_t read_value = *(local_mem + addr);
        *(local_mem + addr + 1) = read_value;
        #endif
        break;
    }
    case add_label:{
        instruction_t label = (instruction_t) *(instr_mem + (pc++));
        word_t test_meta = (word_t) *(instr_mem + (pc++));
        #ifndef __HLS__
        std::string label_string;
        switch(label){
            case short_latency:
                label_string = "short_latency_" + std::to_string(test_meta);
                break;
            case medium_latency:
                label_string = "medium_latency_" + std::to_string(test_meta);
                break;
            case long_latency:
                label_string = "long_latency_" + std::to_string(test_meta);
                break;
            case long_fifo_latency:
                label_string = "long-fifo_latency_" + std::to_string(test_meta);
                break;
            case medium_fifo_latency:
                label_string = "medium-fifo_latency_" + std::to_string(test_meta);
                break;
            case strided_latency:
                label_string = "strided_latency_" + std::to_string(test_meta);
                break;
            case vector_latency:
                label_string = "vector_latency_" + std::to_string(test_meta);
                break;
            case short_throughput:
                label_string = "short_throughput_" + std::to_string(test_meta);
                break;
            case medium_throughput:
                label_string = "medium_throughput_" + std::to_string(test_meta);
                break;
            case long_throughput:
                label_string = "long_throughput_" + std::to_string(test_meta);
                break;
            case long_fifo_throughput:
                label_string = "long-fifo_throughput_" + std::to_string(test_meta);
                break;
            case medium_fifo_throughput:
                label_string = "medium-fifo_throughput_" + std::to_string(test_meta);
                break;
            case strided_throughput:
                label_string = "strided_throughput_" + std::to_string(test_meta);
                break;
            case vector_throughput:
                label_string = "vector_throughput_" + std::to_string(test_meta);
                break;
            default:
                label_string = "null";
                break;
        }
        std::cout << "test: " << label_string << "\n";
        #endif
        break;
    }
    case short_throughput:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        #ifndef __HLS__
        auto timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            kernel.sendShortAM_normal(AMdst, 0xff0, AMhandler, AMargs, handler_args);
            // sleep(0.001);
        }
        // kernel.wait_reply(loopCount);
        #ifndef __HLS__
        kernel.wait_reply(loopCount);
        stop_timer(&kernel, 0xef0, timer);
        #else
        kernel.wait_reply(loopCount, axi_timer);
        stop_timer(&kernel, 0xef0, axi_timer);
        #endif

        #ifndef __HLS__
        timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            kernel.sendShortAM_normal(AMdst, 0xff0, AMhandler, AMargs, handler_args);
            kernel.wait_reply(1);
        }        
        #ifndef __HLS__
        stop_timer(&kernel, 0xef0, timer);
        #else
        stop_timer(&kernel, 0xef0, axi_timer);
        #endif
        break;
    }
    case medium_throughput:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        #ifndef __HLS__
        auto timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            kernel.sendMediumAM_normal(AMdst, 0xff1, AMhandler, AMargs, handler_args, payloadSize, src_addr);
            // sleep(0.001);
        }
        // kernel.wait_reply(loopCount);
        #ifndef __HLS__
        kernel.wait_reply(loopCount);
        stop_timer(&kernel, 0xef1, timer);
        #else
        kernel.wait_reply(loopCount, axi_timer);
        stop_timer(&kernel, 0xef1, axi_timer);
        #endif

        #ifndef __HLS__
        timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            kernel.sendMediumAM_normal(AMdst, 0xff1, AMhandler, AMargs, handler_args, payloadSize, src_addr);
            kernel.wait_reply(1);
        }
        #ifndef __HLS__
        stop_timer(&kernel, 0xef1, timer);
        #else
        stop_timer(&kernel, 0xef1, axi_timer);
        #endif
        break;
    }
    case medium_fifo_throughput:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        word_t j = 0;
        #ifndef __HLS__
        auto timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            kernel.sendMediumAM_normal(AMdst, 0xff2, AMhandler, AMargs, handler_args, payloadSize);
            for (j = 0; j < payloadSize; j+=GC_DATA_BYTES){
                kernel.sendPayload(AMdst, j, j == payloadSize - ((gc_payloadSize_t)GC_DATA_BYTES));
            }
            // sleep(0.001);
        }
        // kernel.wait_reply(loopCount);
        #ifndef __HLS__
        kernel.wait_reply(loopCount);
        stop_timer(&kernel, 0xef2, timer);
        #else
        kernel.wait_reply(loopCount, axi_timer);
        stop_timer(&kernel, 0xef2, axi_timer);
        #endif

        #ifndef __HLS__
        timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            kernel.sendMediumAM_normal(AMdst, 0xff2, AMhandler, AMargs, handler_args, payloadSize);
            for (j = 0; j < payloadSize; j+=GC_DATA_BYTES){
                kernel.sendPayload(AMdst, j, j == payloadSize - ((gc_payloadSize_t)GC_DATA_BYTES));
            }
            kernel.wait_reply(1);
        }
        #ifndef __HLS__
        stop_timer(&kernel, 0xef2, timer);
        #else
        stop_timer(&kernel, 0xef2, axi_timer);
        #endif
        break;
    }
    case long_throughput:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        #ifndef __HLS__
        auto timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            // auto timer2 = start_timer();
            kernel.sendLongAM_normal(AMdst, 0xff3, AMhandler, AMargs, handler_args, payloadSize, src_addr, dst_addr);
            // print_time(timer2, "kernel_send_0");
            // for(int z = 0; z < 10000; z++){
            //     __asm__ __volatile__ ("" : "+g" (z) : : );
            // }
            // sleep(0.001);
        }
        // std::cout << "mem:" << nodedata->mem_ready_barrier_cnt << std::endl;
        // auto timer2 = start_timer();
        // kernel.wait_reply(loopCount);
        #ifndef __HLS__
        kernel.wait_reply(loopCount);
        stop_timer(&kernel, 0xef3, timer);
        #else
        kernel.wait_reply(loopCount, axi_timer);
        stop_timer(&kernel, 0xef3, axi_timer);
        #endif

        #ifndef __HLS__
        timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            // auto timer2 = start_timer();
            kernel.sendLongAM_normal(AMdst, 0xff3, AMhandler, AMargs, handler_args, payloadSize, src_addr, dst_addr);
            // print_time(timer2, "kernel_send_1");
            // timer2 = start_timer();
            kernel.wait_reply(1);
            // print_time(timer2, "kernel_wait_1");
        }
        #ifndef __HLS__
        stop_timer(&kernel, 0xef3, timer);
        #else
        stop_timer(&kernel, 0xef3, axi_timer);
        #endif
        break;
    }
    case long_fifo_throughput:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        word_t j = 0;
        #ifndef __HLS__
        auto timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            // auto timer2 = start_timer();
            kernel.sendLongAM_normal(AMdst, 0xff4, AMhandler, AMargs, handler_args, payloadSize, dst_addr);
            // print_time(timer2, "kernel_send_2_0");
            // timer2 = start_timer();
            for (j = 0; j < payloadSize; j+=GC_DATA_BYTES){
                kernel.sendPayload(AMdst, j, j == payloadSize - ((gc_payloadSize_t)GC_DATA_BYTES));
            }
            // sleep(0.001);
            // print_time(timer2, "kernel_send_2_1");
        }
        // std::cout << "mem:" << nodedata->mem_ready_barrier_cnt << std::endl;
        // auto timer2 = start_timer();
        // kernel.wait_reply(loopCount);
        #ifndef __HLS__
        kernel.wait_reply(loopCount);
        stop_timer(&kernel, 0xef4, timer);
        #else
        kernel.wait_reply(loopCount, axi_timer);
        stop_timer(&kernel, 0xef4, axi_timer);
        #endif

        #ifndef __HLS__
        timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            // auto timer2 = start_timer();
            kernel.sendLongAM_normal(AMdst, 0xff4, AMhandler, AMargs, handler_args, payloadSize, dst_addr);
            // print_time(timer2, "kernel_send_3_0");
            // timer2 = start_timer();
            for (j = 0; j < payloadSize; j+=GC_DATA_BYTES){
                kernel.sendPayload(AMdst, j, j == payloadSize - ((gc_payloadSize_t)GC_DATA_BYTES));
            }
            // print_time(timer2, "kernel_send_3_1");
            // timer2 = start_timer();
            kernel.wait_reply(1);
            // print_time(timer2, "kernel_wait_3");
        }
        #ifndef __HLS__
        stop_timer(&kernel, 0xef4, timer);
        #else
        stop_timer(&kernel, 0xef4, axi_timer);
        #endif
        break;
    }
    case strided_latency:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        for(i = 0; i < loopCount; i++){
            #ifndef __HLS__
            auto timer = start_timer();
            #else
            start_timer(axi_timer);
            #endif
            kernel.sendLongStrideAM_normal(AMdst, 0xff5, AMhandler, AMargs, handler_args, payloadSize, src_stride, src_blk_size,
                src_blk_num, src_addr, dst_stride, dst_blk_size, dst_blk_num, dst_addr);
            // kernel.wait_reply(1);
            #ifndef __HLS__
            kernel.wait_reply(1);
            stop_timer(&kernel, 0xab5, timer);
            #else
            kernel.wait_reply(1, axi_timer);
            stop_timer(&kernel, 0xab5, axi_timer);
            #endif
        }
        break;
    }
    case strided_throughput:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        #ifndef __HLS__
        auto timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            kernel.sendLongStrideAM_normal(AMdst, 0xff5, AMhandler, AMargs, handler_args, payloadSize, src_stride, src_blk_size,
                src_blk_num, src_addr, dst_stride, dst_blk_size, dst_blk_num, dst_addr);
            // sleep(0.001);
        }
        // kernel.wait_reply(loopCount);
        #ifndef __HLS__
        kernel.wait_reply(loopCount);
        stop_timer(&kernel, 0xab5, timer);
        #else
        kernel.wait_reply(loopCount, axi_timer);
        stop_timer(&kernel, 0xab5, axi_timer);
        #endif

        #ifndef __HLS__
        timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            kernel.sendLongStrideAM_normal(AMdst, 0xff5, AMhandler, AMargs, handler_args, payloadSize, src_stride, src_blk_size,
                src_blk_num, src_addr, dst_stride, dst_blk_size, dst_blk_num, dst_addr);
            kernel.wait_reply(1);
        }
        #ifndef __HLS__
        stop_timer(&kernel, 0xab5, timer);
        #else
        stop_timer(&kernel, 0xab5, axi_timer);
        #endif
        break;
    }
    case vector_latency:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        for(i = 0; i < loopCount; i++){
            #ifndef __HLS__
            auto timer = start_timer();
            #else
            start_timer(axi_timer);
            #endif
            kernel.sendLongVectorAM_normal(AMdst, 0xff6, AMhandler, AMargs, handler_args, payloadSize, srcVectorCount, dstVectorCount,
            srcSize, dstSize, src_addrs, dst_addrs);
            // kernel.wait_reply(1);
            #ifndef __HLS__
            kernel.wait_reply(1);
            stop_timer(&kernel, 0xab6, timer);
            #else
            kernel.wait_reply(1, axi_timer);
            stop_timer(&kernel, 0xab6, axi_timer);
            #endif
        }
        break;
    }
    case vector_throughput:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i;
        #ifndef __HLS__
        auto timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            kernel.sendLongVectorAM_normal(AMdst, 0xff6, AMhandler, AMargs, handler_args, payloadSize, srcVectorCount, dstVectorCount,
            srcSize, dstSize, src_addrs, dst_addrs);
            // sleep(0.001);
        }
        // kernel.wait_reply(loopCount);
        #ifndef __HLS__
        kernel.wait_reply(loopCount);
        stop_timer(&kernel, 0xab6, timer);
        #else
        kernel.wait_reply(loopCount, axi_timer);
        stop_timer(&kernel, 0xab6, axi_timer);
        #endif

        #ifndef __HLS__
        timer = start_timer();
        #else
        start_timer(axi_timer);
        #endif
        for(i = 0; i < loopCount; i++){
            kernel.sendLongVectorAM_normal(AMdst, 0xff6, AMhandler, AMargs, handler_args, payloadSize, srcVectorCount, dstVectorCount,
            srcSize, dstSize, src_addrs, dst_addrs);
            kernel.wait_reply(1);
        }
        #ifndef __HLS__
        stop_timer(&kernel, 0xab6, timer);
        #else
        stop_timer(&kernel, 0xab6, axi_timer);
        #endif
        break;
    }
    case wait_counter:{
        word_t value = (word_t) *(instr_mem + (pc++));
        kernel.wait_counter(value);
    }
    case busy_loop:{
        loopCount = (word_t) *(instr_mem + (pc++));
        int i = 0;
        int value = 0;
        while(i < loopCount && value != 1){
            #ifdef __HLS__
            value = (axi_timer[0]) & 0xFFFFFFFF;
            #endif
            i += 1;
        }
        break;
    }
    case send_pilot:{
        gc_AMdst_t dest = (word_t) *(instr_mem + (pc++));
        word_t j = 0;
        kernel.sendMediumAM_normal(dest, 0xbad, AMhandler, AMargs, handler_args, payloadSize);
        for (j = 0; j < payloadSize; j+=GC_DATA_BYTES){
            kernel.sendPayload(AMdst, j, j == payloadSize - ((gc_payloadSize_t)GC_DATA_BYTES));
        }
        kernel.wait_reply(1);
        #ifndef __HLS__
        sleep(1);
        #endif
        break;
    }
    case recv_pilot:{
        axis_word = in->read(); // read token
        axis_word = in->read();
        #ifndef __HLS__
        sleep(1);
        #endif
        break;
    }
    default:
        loop = false;
        break;
    } // switch
    } // while
    kernel.end();
    #ifdef __HLS__
    while(1){}
    #endif
}

#ifndef __HLS__

void kern0(
    short id,
    galapagos::interface <word_t> * in,
    #ifdef __HLS__
    galapagos::interface<word_t> * out,
    int * handler_ctrl
    #else
    galapagos::interface<word_t> * out
    #endif
){

    int numbers[1024];
    // std::string str = "/home/savi/Documents/varun/repos/shoal";
    std::string str = ".";
    // std::string str;
    // str.append(shoal_path);
    str.append("/tests/build/benchmark_0_sw.mem");
    std::ifstream inputFile(str.c_str()); // Input file stream object

    // Check if exists and then open the file.
    if (inputFile.good()) {
        // Push items into a vector
        int current_number = 0;
        std::string first_address;
        inputFile >> first_address;
        int i = 0;
        while (inputFile >> current_number){
            numbers[i] = current_number;
            i++;
        }

        // Close the file.
        inputFile.close();

        benchmark(id, in, out, &numbers[0]);

        std::cout << std::endl;

    } else {
        std::cout << "Error!";
    }
}

#if(KERN_BUILD == -1 || KERN_BUILD == 1)
void kern1(
    short id,
    galapagos::interface <word_t> * in,
    #ifdef __HLS__
    galapagos::interface<word_t> * out,
    int * handler_ctrl
    #else
    galapagos::interface<word_t> * out
    #endif
){

    int numbers[1024];
    // std::string str = "/home/savi/Documents/varun/repos/shoal";
    std::string str = ".";
    // std::string str;
    // str.append(shoal_path);
    str.append("/tests/build/benchmark_1_sw.mem");
    std::ifstream inputFile(str.c_str()); // Input file stream object

    // Check if exists and then open the file.
    if (inputFile.good()) {
        // Push items into a vector
        int current_number = 0;
        std::string first_address;
        inputFile >> first_address;
        int i = 0;
        while (inputFile >> current_number){
            numbers[i] = current_number;
            i++;
        }

        // Close the file.
        inputFile.close();

        benchmark(id, in, out, &numbers[0]);

        std::cout << std::endl;

    } else {
        std::cout << "Error!";
    }
}
#endif

#if(KERN_BUILD == -1 || KERN_BUILD == 2)
void kern2(
    short id,
    galapagos::interface <word_t> * in,
    #ifdef __HLS__
    galapagos::interface<word_t> * out,
    int * handler_ctrl
    #else
    galapagos::interface<word_t> * out
    #endif
){

    int numbers[1024];
    // std::string str = "/home/savi/Documents/varun/repos/shoal";
    std::string str = ".";
    // std::string str;
    // str.append(shoal_path);
    str.append("/tests/build/benchmark_2_sw.mem");
    std::ifstream inputFile(str.c_str()); // Input file stream object

    // Check if exists and then open the file.
    if (inputFile.good()) {
        // Push items into a vector
        int current_number = 0;
        std::string first_address;
        inputFile >> first_address;
        int i = 0;
        while (inputFile >> current_number){
            numbers[i] = current_number;
            i++;
        }

        // Close the file.
        inputFile.close();

        benchmark(id, in, out, &numbers[0]);

        std::cout << std::endl;

    } else {
        std::cout << "Error!";
    }
}
#endif

PGAS_METHOD(kern0, KERN0_ID)
#if(KERN_BUILD == -1 || KERN_BUILD == 1)
PGAS_METHOD(kern1, KERN1_ID)
#endif
#if(KERN_BUILD == -1 || KERN_BUILD == 2)
PGAS_METHOD(kern2, KERN2_ID)
#endif

#endif
}
