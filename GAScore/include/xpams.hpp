#ifndef XPAMS_H_
#define XPAMS_H_

#include "GAScore.hpp"
#define DEBUG
#ifdef DEBUG
#include "shoal_testbench.hpp"
#endif

#define FSM_EXISTS // allows printing FSM states in debug

#define H_EMPTY 0

static enum state_t{st_AMheader, st_sendReplyHeader, 
    st_AMpayload} currentState;

#define DECLARE_VARIABLES\
    axis_t axis_rx("rx");\
    axis_t axis_tx("tx");\
    axis_dest_t axis_kernel_out("kernel_out");\
\
    axis_word_t axis_word;\
    axis_wordDest_t axis_wordDest;\
\
    uint_64_t readData;\
    uint_1_t readLast;

#ifdef DEBUG
#define CALL_TB xpams(dbg_currentState, axis_rx, axis_tx, \
    axis_kernel_out);
#else
#define CALL_TB xpams(axis_rx, axis_tx,axis_kernel_out);
#endif

#define PRINT_INTERFACES std::cout << "Stream statuses:\n"; \
    PRINT_AXIS_SIZE("RX", axis_rx) \
    PRINT_AXIS_SIZE("TX", axis_tx) \
    PRINT_AXIS_SIZE("Kernel_Out", axis_kernel_out)

#define READ_INTERFACES \
    READ_STREAM_INTERFACE("RX", uaxis_l, axis_rx, axis_word)\
    READ_STREAM_INTERFACE("TX", uaxis_l, axis_tx, axis_word)\
    READ_STREAM_INTERFACE("Kernel_Out", uaxis_m, axis_kernel_out, axis_wordDest)\

inline axis_wordDest_t assignWord(axis_word_t axis_word);

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif