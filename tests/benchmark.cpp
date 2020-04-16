// #ifndef __HLS__
// #define __HLS__
// #endif
#include <cstddef> // needed to resolve ::max_align_t errors
#include <cstring>
#include "benchmark.hpp"

#ifndef __HLS__
#include <iostream>
#include <fstream> // To use ifstream
#endif

enum instruction_t{
    load, short_latency, medium_latency, long_latency, barrier_send,
    recv_medium, end, barrier_wait, long_fifo_latency, medium_fifo_latency,
    read_local, recv_time, add_label}
    current_instruction;

#ifndef __HLS__
auto start_timer(){
    return std::chrono::high_resolution_clock::now();
}

void stop_timer(shoal::kernel* kernel, gc_AMToken_t token, std::chrono::high_resolution_clock::time_point timer){
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - timer;
    word_t time = (word_t)(elapsed.count() * 1E9 / 6.4); // convert to 156.25 MHz (6.4ns) cycles
    kernel->sendMediumAM_async(0, token, H_EMPTY, 0, NULL, 8);
    kernel->sendPayload(0, time, true); // assume always send to kernel 0
}
#else
// this function, if inlined, or placed in code, behaves badly in Verilog. In
// simulation, it does do three writes but they appear as burst writes to the wrong
// addresses. Doing it this way seems to work properly. Tested in HLS 2018.1
void start_timer(volatile int* axi_timer){
    axi_timer[1] = 0; // set load register to 0
    axi_timer[0] = 0x20; // load timer with load register
    axi_timer[0] = 0x80; // start timer
}

void stop_timer(shoal::kernel* kernel, gc_AMToken_t token, volatile int* axi_timer){
    #pragma HLS INLINE
    axi_timer[0] = 0x0; // stop timer
    word_t time = *(axi_timer + 0x2); // read timer count
    kernel->sendMediumAM_async(0, token, H_EMPTY, 0, NULL, 8);
    kernel->sendPayload(0, time, true); // assume always send to kernel 0
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
            kernel.wait_reply(1);
            #ifndef __HLS__
            stop_timer(&kernel, 0xab0, timer);
            #else
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
            kernel.wait_reply(1);
            #ifndef __HLS__
            stop_timer(&kernel, 0xab1, timer);
            #else
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
                kernel.sendPayload(AMdst, j, j == payloadSize - GC_DATA_BYTES);
            }
            kernel.wait_reply(1);
            #ifndef __HLS__
            stop_timer(&kernel, 0xab2, timer);
            #else
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
            kernel.wait_reply(1);
            #ifndef __HLS__
            stop_timer(&kernel, 0xab3, timer);
            #else
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
                kernel.sendPayload(AMdst, j, j == payloadSize - GC_DATA_BYTES);
            }
            kernel.wait_reply(1);
            #ifndef __HLS__
            stop_timer(&kernel, 0xab4, timer);
            #else
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
                label_string = "long_fifo_latency_" + std::to_string(test_meta);
                break;
            case medium_fifo_latency:
                label_string = "medium_fifo_latency_" + std::to_string(test_meta);
                break;
            default:
                label_string = "null";
                break;
        }
        std::cout << "test: " << label_string << "\n";
        #endif
        break;
    }
    default:
        loop = false;
        break;
    }
    }
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
    std::ifstream inputFile("/home/savi/Documents/varun/repos/shoal/tests/build/benchmark_0_sw.mem");        // Input file stream object

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
    std::ifstream inputFile("/home/savi/Documents/varun/repos/shoal/tests/build/benchmark_1_sw.mem");        // Input file stream object

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
    std::ifstream inputFile("/home/savi/Documents/varun/repos/shoal/tests/build/benchmark_2_sw.mem");        // Input file stream object

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
