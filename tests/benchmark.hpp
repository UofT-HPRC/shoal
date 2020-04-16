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

#define KERNEL_NUM_TOTAL 3
#define KERN0_ID 0
#define KERN1_ID 1
#define KERN2_ID 2

// in bytes
#define SEGMENT_SIZE 16384
