#if !defined(HANDLER_H_)
#define HANDLER_H_

#include "utilities.hpp"
// #define DEBUG
#ifdef DEBUG
#include "testbench.hpp"
#endif

#define FSM_EXISTS // allows printing FSM states in debug

static enum state_rx_t{st_AMHeader, st_handler} currentState;

#define PREV_TOKENS 3

#define DECLARE_VARIABLES\
    axis_noKeep_t axis_handler("handler");\
\
    axis_wordNoKeep_t axis_word;\
    gc_AMhandler_t AMhandler;\
    uint_32_t counter_threshold = 2;\
    uint_1_t counter_reset = 1;\
    uint_1_t interrupt;\
\
    uint_64_t readData;\
    uint_1_t readLast;

#ifdef DEBUG
#define CALL_TB handler(axis_handler, AMhandler, counter_threshold, counter_reset, \
    interrupt);
#else
#define CALL_TB handler(axis_handler, AMhandler, counter_threshold, counter_reset, \
    interrupt);
#endif

#define PRINT_INTERFACES std::cout << "Stream statuses:\n"; \
    PRINT_AXIS_SIZE("Handler", axis_handler)

#define READ_INTERFACES \
    READ_STREAM_INTERFACE("Handler", uaxis_n, axis_handler, axis_word)

// void handler(
//     axis_noKeep_t &axis_handler, //input to handler
//     gc_AMhandler_t AMhandler,
//     uint_32_t counter_threshold,
//     uint_2_t counter_reset,
//     uint_1_t &interrupt
// );
void handler(
    axis_noKeep_t &axis_handler, //input to handler
    gc_AMhandler_t AMhandler,
    uint_32_t config,
    ap_int<32> arg,
    ap_int<32> &counter_out,
    ap_int<32> &barrier_out,
    ap_int<32> &mem_ready_out
);

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif // HANDLER_H_
