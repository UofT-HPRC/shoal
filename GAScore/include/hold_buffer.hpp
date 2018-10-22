#ifndef HOLDBUFFER_H_
#define HOLDBUFFER_H_

#include "GAScore.hpp"
// #define DEBUG
#ifdef DEBUG
#include "shoal_testbench.hpp"
#endif

#define FIFO_DEPTH 512
#define LOG_FIFO_DEPTH 9

typedef ap_uint<LOG_FIFO_DEPTH> count_t;

#define DECLARE_VARIABLES\
    axis_t axis_input("input");\
    axis_t axis_output("output");\
    uint_1_t dataRelease;\
\
    axis_word_t axis_word;\
\
    uint_64_t readData;\
    uint_1_t readLast;

#define CALL_TB hold_buffer(axis_input, axis_output, dataRelease);

#define PRINT_INTERFACES std::cout << "Stream statuses:\n"; \
    PRINT_AXIS_SIZE("Input", axis_input) \
    PRINT_AXIS_SIZE("Output", axis_output) 
    
#define READ_INTERFACES \
    READ_STREAM_INTERFACE("Input", uaxis_l, axis_input, axis_word)\
    READ_STREAM_INTERFACE("Output", uaxis_l, axis_output, axis_word)

#endif // HOLDBUFFER_H_
