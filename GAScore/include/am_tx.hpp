#ifndef AM_TX_H_
#define AM_TX_H_

#include "utilities.hpp"
// #define DEBUG
#ifdef DEBUG
#include "testbench.hpp"
#endif

#define FSM_EXISTS // allows printing FSM states in debug

enum state_t{st_header, st_AMHandlerArgs,
    st_AMLongVector, st_AMdestination, st_AMToken,
    st_AMpayload, st_AMLongStride, st_done, st_AMsource, 
    st_AMforward, st_AMLongStride_1, st_AMLongStride_2, st_AMLongStride_3,
    st_AMLongVector_1, st_AMLongVector_2, st_AMLongVectorSrcRead,
    st_AMLongVectorSrcRead_1, st_AMLongVectorDstRead, st_AMLongVectorDstRead_1,
    };

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
    READ_STREAM_INTERFACE("mm2sCommand", uaxis_o, axis_mm2sCommand, axis_word_mm2sCommand)\
    READ_STREAM_INTERFACE("mm2s", uaxis_l, axis_mm2s, axis_word)\
    READ_STREAM_INTERFACE("mm2sStatus", uaxis_l, axis_mm2sStatus, axis_word_mm2sStatus)

void am_tx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_kernel, //input
    axis_dest_t &axis_net, //output
    dataMoverCommand_t &axis_mm2sCommand, //output
    axis_t &axis_mm2s, //input
    dataMoverStatus_t &axis_mm2sStatus, //input
    uint_1_t &release //output
);

inline void write(axis_dest_t &axis_net, axis_word_t axis_word, gc_AMdst_t dest){
    #pragma HLS INLINE
    axis_wordDest_t axis_word_dest;
    axis_word_dest.data = axis_word.data;
    axis_word_dest.last = axis_word.last;
    axis_word_dest.keep = axis_word.keep;
    axis_word_dest.dest = dest;
    axis_net.write(axis_word_dest);
}

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif
