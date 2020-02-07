#pragma once

#ifndef __HLS__
#define __HLS__
#endif
#include "active_messages.hpp"
#include "shoal_kernel.hpp"

#define INSTR_MEMORY_OFFSET 0x1000
#define AXI_TIMER_OFFSET 0x2000
#define MEMORY_OFFSET 0x3000
#define KERNEL_NUM_TOTAL 2

enum instruction_t{
    load, send_short, send_medium, send_long, start_timer, stop_timer,
    barrier_send, recv_medium, end, barrier_wait, send_long_fifo, send_medium_fifo,
    read_local}
    current_instruction;

extern "C" void hls_kernel(
    short id,
    galapagos::interface <word_t> * in,
    galapagos::interface<word_t> * out,
    volatile int * handler_ctrl,
    volatile int * axi_timer,
    word_t * instr_mem,
    word_t * local_mem
);
