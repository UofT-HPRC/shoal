#ifndef XPAMS_TX_H_
#define XPAMS_TX_H_

#include "utilities.hpp"
// #define DEBUG
#ifdef DEBUG
#include "testbench.hpp"
#endif

#define FSM_EXISTS // allows printing FSM states in debug

static enum state_rx_t{st_AMheader, st_AMsend, st_AMargs,
    st_AMpayload, st_AMloopback, st_AMtoken, st_AMreply} currentState;

#define DECLARE_VARIABLES\
    axis_t axis_kernel_in("kernel_in");\
    axis_t axis_tx("tx");\
    axis_dest_t axis_kernel_out("kernel_out");\
    axis_noKeep_t axis_handler("handler");\
\
    axis_word_t axis_word;\
    axis_wordDest_t axis_wordDest;\
    axis_wordNoKeep_t axis_wordNoKeep;\
    const gc_AMdest_t address_offset_low = 0;\
    const gc_AMdest_t address_offset_high = 16;\
\
    uint_64_t readData;\
    uint_1_t readLast;

#ifdef DEBUG
#define CALL_TB xpams_tx(dbg_currentState, axis_kernel_in, axis_kernel_out, \
    axis_tx, axis_handler, address_offset_low, address_offset_high);
#else
#define CALL_TB xpams_tx(axis_kernel_in, axis_kernel_out,axis_tx, axis_handler,\
    address_offset_low, address_offset_high);
#endif

#define PRINT_INTERFACES std::cout << "Stream statuses:\n"; \
    PRINT_AXIS_SIZE("Kernel_In", axis_kernel_in) \
    PRINT_AXIS_SIZE("TX", axis_tx) \
    PRINT_AXIS_SIZE("Kernel_Out", axis_kernel_out)\
    PRINT_AXIS_SIZE("Handler", axis_handler)

#define READ_INTERFACES \
    READ_STREAM_INTERFACE("Kernel_In", uaxis_l, axis_kernel_in, axis_word)\
    READ_STREAM_INTERFACE("TX", uaxis_l, axis_tx, axis_word)\
    READ_STREAM_INTERFACE("Kernel_Out", uaxis_m, axis_kernel_out, axis_wordDest)\
    READ_STREAM_INTERFACE("Handler", uaxis_n, axis_handler, axis_wordNoKeep)

void xpams_tx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_kernel_in, //input from kernel
    axis_dest_t &axis_kernel_out, //output to kernel
    axis_t &axis_tx, // output to am_tx (from kernel)
    axis_noKeep_t &axis_handler, //output to handler

    const gc_AMdest_t address_offset_low,
    const gc_AMdest_t address_offset_high
);

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif
