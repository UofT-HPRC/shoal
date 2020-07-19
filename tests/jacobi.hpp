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

#define KERNEL_NUM_TOTAL 5
#define KERN0_ID 0
#define KERN1_ID 1
#define KERN2_ID 2
#define KERN3_ID 3
#define KERN4_ID 4

// in bytes
#define SEGMENT_SIZE 33554432
typedef std::chrono::high_resolution_clock::time_point time_point_t;

// THeGASnet compatibility things
typedef gc_AMToken_t gasnet_token_t;
typedef word_t gasnet_handlerarg_t;
typedef unsigned long long voidp64;
#define PRINT(...) printf(__VA_ARGS__)

// #define COMPILER_BARRIER __sync_synchronize();
#define COMPILER_BARRIER

// formerly arguments, now compile-time values
#define GRID_SIZE 1024