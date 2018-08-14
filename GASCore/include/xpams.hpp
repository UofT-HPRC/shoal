#ifndef XPAMS_H_
#define XPAMS_H_

#include "gascore.hpp"
// #define DEBUG

#define H_EMPTY 0
#define H_INCR 1

typedef uint_32_t counter_t;

/*
    increment: | Index (1:0) | Value (3:2)| Reserved (31:4)
    enable: | AMCounter_reset | wordCounter_reset | AMcounter_master | 
        wordCounter_master | customCounter (7:4) | handlerCounter (11:8) | 
        customCounter_reset (15:12) | handlerCounter_reset (19:16)
*/

#define COUNTER_NUM 4
typedef uint_2_t counterIndex_t;
typedef uint_2_t counterValue_t;
static enum state_t{st_idle, st_AMheader, st_increment, st_sendReplyHeader, 
    st_AMpayload, st_error} currentState;
void xpams(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_rx, //input from GASCore
    axis_t &axis_tx_handler, //output AM reply
    axis_t &axis_kernel_out, //output data to kernel
    axis_t &axis_kernel_in, //input data from kernel
    axis_t &axis_tx_kernel, //output AM from kernel
    uint_1_t &blockingWait,
    counter_t AMcounter_threshold,
    counter_t wordCounter_threshold,
    counter_t customCounter_master[COUNTER_NUM],
    counter_t handlerCounter_threshold[COUNTER_NUM],
    counter_t handlerCounter_master[COUNTER_NUM],
    word_t enable,
    word_t mask
);

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif