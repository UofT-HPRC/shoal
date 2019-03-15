#ifndef AM_RX_H_
#define AM_RX_H_

#include "utilities.hpp"
#ifdef DEBUG
#include "testbench.hpp"
#endif

#define FSM_EXISTS // allows printing FSM states in debug

static enum state_t{st_header, st_AMHandlerArgs, st_AMLongVector, 
    st_AMdestination, st_AMToken, st_AMpayload, st_AMLongStride, st_done} 
    currentState;

#define DECLARE_VARIABLES\
    axis_t axis_xpams_rx("xpams_rx");\
    axis_t axis_net("net");\
    dataMoverCommand_t axis_s2mmCommand("s2mmCommand");\
    axis_t axis_s2mm("s2mm");\
    dataMoverStatus_t axis_s2mmStatus("s2mmStatus");\
\
    uint_1_t release;\
    gc_AMdest_t destOffset = 0;\
\
    axis_word_t axis_word;\
    dataMoverCommand_word_t axis_word_s2mmCommand;\
    axis_word_8a_t axis_word_s2mmStatus;\
\
    uint_64_t readData;\
    uint_1_t readLast;

#ifdef DEBUG
#define CALL_TB am_rx(dbg_currentState, axis_xpams_rx, axis_net, \
    axis_s2mmCommand,axis_s2mm, axis_s2mmStatus, release, destOffset);
#else
#define CALL_TB am_rx(axis_xpams_rx, axis_net,axis_s2mmCommand, \
    axis_s2mm, axis_s2mmStatus, release);
#endif

#define PRINT_INTERFACES std::cout << "Stream statuses:\n"; \
    PRINT_AXIS_SIZE("xpams_rx", axis_xpams_rx) \
    PRINT_AXIS_SIZE("Network", axis_net) \
    PRINT_AXIS_SIZE("s2mmCommand", axis_s2mmCommand) \
    PRINT_AXIS_SIZE("s2mm", axis_s2mm) \
    PRINT_AXIS_SIZE("s2mmStatus", axis_s2mmStatus)

#define READ_INTERFACES \
    READ_STREAM_INTERFACE("xpams_rx", uaxis_l, axis_xpams_rx, axis_word)\
    READ_STREAM_INTERFACE("Net", uaxis_l, axis_net, axis_word)\
    READ_STREAM_INTERFACE("s2mmCommand", uaxis_l, axis_s2mmCommand, axis_word_s2mmCommand)\
    READ_STREAM_INTERFACE("s2mm", uaxis_l, axis_s2mm, axis_word)\
    READ_STREAM_INTERFACE("s2mmStatus", uaxis_l, axis_s2mmStatus, axis_word_s2mmStatus)

void am_rx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_xpams_rx, //output
    axis_t &axis_net, //input
    dataMoverCommand_t &axis_s2mmCommand, //output
    axis_t &axis_s2mm, //output
    dataMoverStatus_t &axis_s2mmStatus, //input

    //axis_xpams_rx release
    uint_1_t &release //output
);

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif
