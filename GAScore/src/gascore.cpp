#include "GAScore.hpp"

void GAScore(
    axis_t &axis_net_in, //input
    axis_t &axis_net_out, //output
    axis_t &axis_kernel_in, //input
    axis_dest_t &axis_kernel_out, //output

    dataMoverCommand_t &axis_s2mmCommand, // output
    dataMoverCommand_t &axis_mm2sCommand, //output
    axis_t &axis_s2mm, // output
    axis_t &axis_mm2s, // input
    dataMoverStatus_t &axis_s2mmStatus, // input
    dataMoverStatus_t &axis_mm2sStatus, // input

    gc_AMdest_t destOffset
){
    #pragma HLS INTERFACE axis port=axis_net_in
    #pragma HLS INTERFACE axis port=axis_net_out
    #pragma HLS INTERFACE axis port=axis_kernel_in
    #pragma HLS INTERFACE axis port=axis_kernel_out
    #pragma HLS INTERFACE axis port=axis_s2mmCommand
    #pragma HLS INTERFACE axis port=axis_mm2sCommand
    #pragma HLS INTERFACE axis port=axis_s2mm
    #pragma HLS INTERFACE axis port=axis_mm2s
    #pragma HLS INTERFACE axis port=axis_s2mmStatus
    #pragma HLS INTERFACE axis port=axis_mm2sStatus
    #pragma HLS INTERFACE ap_stable port=destOffset
    #pragma HLS DATAFLOW

    uint_1_t am_rx_release;
    uint_1_t am_tx_release;
    axis_t axis_rx_hold;
    axis_t axis_tx_hold;
    axis_t axis_hold_xpams;
    axis_t axis_xpams_tx;

    am_rx(axis_rx_hold, axis_net_in, axis_s2mmCommand, axis_s2mm, 
        axis_s2mmStatus, am_rx_release, destOffset);
    hold_buffer(axis_rx_hold, axis_hold_xpams, am_rx_release);
    am_tx(axis_xpams_tx, axis_tx_hold, axis_mm2sCommand, axis_mm2s, 
        axis_mm2sStatus, am_tx_release);
    xpams(axis_hold_xpams, axis_xpams_tx, axis_kernel_out, axis_kernel_in);
    hold_buffer(axis_tx_hold, axis_net_out, am_tx_release);
    
}