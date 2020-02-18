// #ifndef __HLS__
// #define __HLS__
// #endif
#include <cstddef> // needed to resolve ::max_align_t errors
#include <cstring>
#include "hls_kernel.hpp"

#ifndef __HLS__
#include <iostream>
#include <fstream> // To use ifstream
#endif

enum instruction_t{
    load, send_short, send_medium, send_long, start_timer, stop_timer,
    barrier_send, recv_medium, end, barrier_wait, send_long_fifo, send_medium_fifo,
    read_local}
    current_instruction;

// this function, if inlined, or placed in code, behaves badly in Verilog. In
// simulation, it does do three writes but they appear as burst writes to the wrong
// addresses. Doing it this way seems to work properly. Tested in HLS 2018.1
void start_timer_func(volatile int* axi_timer){
    axi_timer[1] = 0; // set load register to 0
    axi_timer[0] = 0x10; // load timer with load register
    axi_timer[0] = 0x80; // start timer
}

extern "C"{
void hls_kernel(
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
    auto timer = std::chrono::high_resolution_clock::now();
    kernel.attach(nullptr, 0, SEGMENT_SIZE);
    #endif
    galapagos::stream_packet <word_t> axis_word;
    int pc = 0;

    gc_AMdst_t AMdst;
    gc_AMToken_t AMtoken;
    gc_AMhandler_t AMhandler;
    gc_AMargs_t AMargs;
    word_t handler_args [16];
    gc_payloadSize_t payloadSize;
    gc_AMsrc_t src_addr;
    gc_AMdest_t dst_addr;
    int i;
    short addr;

    instruction_t instruction;
    bool loop = true;

    while(loop){
    instruction = (instruction_t) *(instr_mem + pc++);
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
        #ifdef __HLS__
        start_timer_func(axi_timer);
        #else
        timer = std::chrono::high_resolution_clock::now();
        #endif
        break;
    }
    case stop_timer:{
        #ifdef __HLS__
        axi_timer[0] = 0x0; // stop timer
        word_t time = *(axi_timer + 0x2); // read timer count
        // gc_AMdst_t AMdst_tmp = (instruction_t) *(instr_mem + (pc++));
        #else
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - timer;
        word_t time = (word_t)(elapsed.count() * 1E9); // convert to ns
        #endif
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
        #ifdef __HLS__
        word_t addr = (word_t) *(instr_mem + (pc++));
        word_t read_value = *(local_mem + addr);
        *(local_mem + addr + 1) = read_value;
        #endif
        break;
    }
    // case end:{
    //     while(1){}
    //     break;
    // }
    default:
        loop = false;
        break;
    }
    }
    kernel.end();
}

#ifndef __HLS__

// void kern0(
//     short id,
//     galapagos::interface <word_t> * in,
//     #ifdef __HLS__
//     galapagos::interface<word_t> * out,
//     int * handler_ctrl
//     #else
//     galapagos::interface<word_t> * out
//     #endif
// ){

//     int numbers[256];
//     std::ifstream inputFile("/home/savi/Documents/varun/repos/shoal/tests/hls_kernel_0.mem");        // Input file stream object

//     // Check if exists and then open the file.
//     if (inputFile.good()) {
//         // Push items into a vector
//         int current_number = 0;
//         std::string first_address;
//         inputFile >> first_address;
//         int i = 0;
//         while (inputFile >> current_number){
//             numbers[i] = current_number;
//             i++;
//         }

//         // Close the file.
//         inputFile.close();

//         // std::cout << "The numbers are: ";
//         // for (unsigned int count = 0; count < numbers.size(); count++){
//         //     std::cout << numbers[count] << " ";

//         // }
//         hls_kernel(id, in, out, &numbers[0]);
//         // hls_kernel(id, in, out, &numbers[2]);
//         // hls_kernel(id, in, out, &numbers[10]);
//         // hls_kernel(id, in, out, &numbers[12]);
//         // hls_kernel(id, in, out, &numbers[13]);

//         std::cout << std::endl;

//     } else {
//         std::cout << "Error!";
//     }
// }

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

    int numbers[256];
    std::ifstream inputFile("/home/savi/Documents/varun/repos/shoal/tests/hls_kernel_1.mem");        // Input file stream object

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

        // std::cout << "The numbers are: ";
        // for (unsigned int count = 0; count < numbers.size(); count++){
        //     std::cout << numbers[count] << " ";

        // }
        hls_kernel(id, in, out, &numbers[0]);
        // hls_kernel(id, in, out, &numbers[1]);
        // hls_kernel(id, in, out, &numbers[9]);
        // hls_kernel(id, in, out, &numbers[10]);
        // hls_kernel(id, in, out, &numbers[11]);
        // hls_kernel(id, in, out, &numbers[12]);
        // hls_kernel(id, in, out, &numbers[14]);

        std::cout << std::endl;

    } else {
        std::cout << "Error!";
    }
}

// PGAS_METHOD(kern0, KERN0_ID)
PGAS_METHOD(kern1, KERN1_ID)

#endif
}
