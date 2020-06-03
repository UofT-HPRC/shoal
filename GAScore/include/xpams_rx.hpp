#ifndef XPAMS_RX_H_
#define XPAMS_RX_H_

#include "utilities.hpp"
// #define DEBUG
#ifdef DEBUG
#include "testbench.hpp"
#endif

#define FSM_EXISTS // allows printing FSM states in debug

enum state_t{st_AMheader, st_sendReplyHeader, st_AMToken,
    st_AMpayload, st_AMforward, st_AMtoken_2, st_AMhandlerArgs,
    st_sendReplyHeader_2};

#define DECLARE_VARIABLES\
    axis_t axis_rx("rx");\
    axis_t axis_tx("tx");\
    axis_noKeep_t axis_handler("handler");\
    axis_dest_t axis_kernel_out("kernel_out");\
\
    axis_word_t axis_word;\
    axis_wordDest_t axis_wordDest;\
    axis_wordNoKeep_t axis_wordNoKeep;\
\
    uint_64_t readData;\
    uint_1_t readLast;

#ifdef DEBUG
#define CALL_TB xpams_rx(dbg_currentState, axis_rx, axis_tx, axis_handler, \
    axis_kernel_out);
#else
#define CALL_TB xpams_rx(axis_rx, axis_tx, axis_handler, axis_kernel_out);
#endif

#define PRINT_INTERFACES std::cout << "Stream statuses:\n"; \
    PRINT_AXIS_SIZE("RX", axis_rx) \
    PRINT_AXIS_SIZE("TX", axis_tx) \
    PRINT_AXIS_SIZE("Kernel_Out", axis_kernel_out)

#define READ_INTERFACES \
    READ_STREAM_INTERFACE("RX", uaxis_l, axis_rx, axis_word)\
    READ_STREAM_INTERFACE("TX", uaxis_l, axis_tx, axis_word)\
    READ_STREAM_INTERFACE("Kernel_Out", uaxis_m, axis_kernel_out, axis_wordDest)\

// extern inline axis_wordDest_t assignWord(axis_word_t axis_word);
// extern inline axis_wordNoKeep_t assignWordtoNoKeep(axis_word_t axis_word);

void xpams_rx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_rx, //input from GAScore
    axis_t &axis_tx, //output AM reply
    axis_noKeep_t &axis_handler, //output to handler
    axis_dest_t &axis_kernel_out //output data to kernel
);

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif
