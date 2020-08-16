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

#ifndef KERNEL_NUM_TOTAL
#define KERNEL_NUM_TOTAL 9
#endif
#ifndef NODES
#define NODES 4
#endif

// in bytes
#define SEGMENT_SIZE 33554432
#ifndef __HLS__
typedef std::chrono::high_resolution_clock::time_point time_point_t;
#endif
#ifndef ITERATIONS
#define ITERATIONS 1025
#endif

// THeGASnet compatibility things
typedef gc_AMToken_t gasnet_token_t;
typedef word_t gasnet_handlerarg_t;
typedef unsigned long long voidp64;
#ifdef __HLS__
#define PRINT(...)
#else
#define PRINT(...) printf(__VA_ARGS__)
#endif

#ifdef __HLS__
#define COMPILER_BARRIER
#else
#define COMPILER_BARRIER __sync_synchronize();
#endif

// formerly arguments, now compile-time values
#ifndef GRID_SIZE
#define GRID_SIZE 1024
#endif

