#ifndef AM_TX_H_
#define AM_TX_H_

#include "GAScore.hpp"
#define DEBUG
#ifdef DEBUG
#include "shoal_testbench.hpp"
#endif

#define FSM_EXISTS // allows printing FSM states in debug

enum state_t{st_header, st_AMHandlerArgs,
    st_AMLongVector, st_AMdestination, st_AMToken,
    st_AMpayload, st_AMLongStride, st_done, st_AMsource};

#define DECLARE_VARIABLES\
    axis_t axis_kernel("kernel");\
    axis_t axis_net("net");\
    dataMoverCommand_t axis_mm2sCommand("mm2sCommand");\
    axis_t axis_mm2s("mm2s");\
    dataMoverStatus_t axis_mm2sStatus("mm2sStatus");\
\
    uint_1_t release;\
\
    axis_word_t axis_word;\
    dataMoverCommand_word_t axis_word_mm2sCommand;\
    axis_word_8a_t axis_word_mm2sStatus;\
\
    uint_64_t readData;\
    uint_1_t readLast;

#ifdef DEBUG
#define CALL_TB am_tx(dbg_currentState, axis_kernel, axis_net, \
    axis_mm2sCommand,axis_mm2s, axis_mm2sStatus, release);
#else
#define CALL_TB am_tx(axis_kernel, axis_net,axis_mm2sCommand, \
    axis_mm2s, axis_mm2sStatus, release);
#endif

#define PRINT_INTERFACES std::cout << "Stream statuses:\n"; \
    PRINT_AXIS_SIZE("Kernel", axis_kernel) \
    PRINT_AXIS_SIZE("Network", axis_net) \
    PRINT_AXIS_SIZE("mm2sCommand", axis_mm2sCommand) \
    PRINT_AXIS_SIZE("mm2s", axis_mm2s) \
    PRINT_AXIS_SIZE("mm2sStatus", axis_mm2sStatus)

#define READ_INTERFACES \
    READ_STREAM_INTERFACE("Kernel", uaxis_l, axis_kernel, axis_word)\
    READ_STREAM_INTERFACE("Network", uaxis_l, axis_net, axis_word)\
    READ_STREAM_INTERFACE("mm2sCommand", uaxis_l, axis_mm2sCommand, axis_word_mm2sCommand)\
    READ_STREAM_INTERFACE("mm2s", uaxis_l, axis_mm2s, axis_word)\
    READ_STREAM_INTERFACE("mm2sStatus", uaxis_l, axis_mm2sStatus, axis_word_mm2sStatus)

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif