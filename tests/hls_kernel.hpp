#pragma once

// #ifndef __HLS__
// #define __HLS__
// #endif
#include "active_messages.hpp"
#ifdef __HLS__

#else // x86 or MB
#include "am_globals.hpp"
#include "am_gasnet.hpp"
#include "shoal_node.hpp"
#endif
#include "shoal_kernel.hpp"

#define KERNEL_NUM_TOTAL 2
#define KERN0_ID 0
#define KERNEL_NUM_0 1
#define KERN1_ID 1
#define KERNEL_NUM_1 1
#define SEGMENT_SIZE 256


extern "C" void hls_kernel(
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
);

extern "C" void kern0(
    short id,
    galapagos::interface <word_t> * in,
    #ifdef __HLS__
    galapagos::interface<word_t> * out,
    int * handler_ctrl
    #else
    galapagos::interface<word_t> * out
    #endif
);
