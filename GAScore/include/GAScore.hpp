#ifndef GASCORE_H_
#define GASCORE_H_

#include "utilities.hpp"

#define DEBUG

void am_rx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_handler, //output
    axis_t &axis_net, //input
    dataMoverCommand_t &axis_s2mmCommand, //output
    axis_t &axis_s2mm, //output
    dataMoverStatus_t &axis_s2mmStatus, //input

    //axis_handler release
    uint_1_t &release, //output
    gc_AMdest_t destOffset //input
);

void am_tx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_kernel, //input
    axis_t &axis_net, //output
    dataMoverCommand_t &axis_mm2sCommand, //output
    axis_t &axis_mm2s, //input
    dataMoverStatus_t &axis_mm2sStatus, //input
    uint_1_t &release //output
);

void xpams(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_rx, //input from GAScore
    axis_t &axis_tx, //output AM reply
    axis_dest_t &axis_kernel_out //output data to kernel
);

void hold_buffer(
    axis_t &axis_input, //input from GAScore
    axis_t &axis_output, //output AM reply
    uint_1_t &dataRelease
);

#endif // GASCORE_H_
