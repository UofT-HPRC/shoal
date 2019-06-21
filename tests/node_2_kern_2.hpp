#if !defined(SHOAL_TEST_NODE_2_KERN_2_H_)
#define SHOAL_TEST_NODE_2_KERN_2_H_

// #define USE_APUINT
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

// extern lock_t lock_node;

#endif // SHOAL_TEST_NODE_2_KERN_2_H_
