#ifndef GASCORE_H_
#define GASCORE_H_

#include "shoal_stream.hpp"
#include "utilities.hpp"

/*******************************************************************************
 * Defines
*******************************************************************************/

#define GC_DATA_WIDTH 64UL // bit width of tdata
#define GC_ADDR_WIDTH 32 // bit width of addressable memory on a node
#define GC_DEST_WIDTH 4 // bit width of tdest
#define GC_MAX_PAYLOAD 12 // Log2 of the maximum payload word count
#define MAX_VECTOR_NUM 16 // Maximum number of vectors for vectored messages

/*******************************************************************************
 * Derived defines
*******************************************************************************/

#define GC_DATA_BYTES (GC_DATA_WIDTH/8)
#define GC_MAX_PAYLOAD_BYTES (GC_MAX_PAYLOAD+NBITS(GC_DATA_BYTES)-1)

/*******************************************************************************
 * Typedefs
*******************************************************************************/

enum type_t{AM_SHORT, AM_MEDIUM_MEM, AM_MEDIUM_FIFO, AM_LONG_MEM, AM_LONG_FIFO, 
    AM_VECTOR, AM_STRIDE} AM_type;

typedef uaxis_l<GC_DATA_WIDTH> axis_word_t;
typedef hls::stream<axis_word_t> axis_t;
typedef uaxis_m<GC_DATA_WIDTH, GC_DEST_WIDTH> axis_wordDest_t;
typedef hls::stream<axis_wordDest_t> axis_dest_t;
typedef ap_uint<GC_DATA_WIDTH> word_t;
typedef ap_uint<GC_ADDR_WIDTH> addr_word_t;

typedef ap_uint<GC_DEST_WIDTH> gc_AMdest_t;

// Header Types
typedef uint_16_t gc_AMsrc_t;
typedef uint_16_t gc_AMdst_t;

// AM Types
typedef uint_8_t gc_AMtype_t;
typedef uint_8_t gc_AMargs_t;
typedef uint_4_t gc_AMhandler_t;
typedef uint_24_t gc_AMToken_t;

// AM Handler args
typedef uint_64_t gc_AMhandlerArg_t;

// Payload
typedef ap_uint<GC_MAX_PAYLOAD> gc_payloadSize_t;
typedef uint_4_t gc_dstVectorNum_t;

// Long
typedef uint_64_t gc_destination_t;
typedef gc_payloadSize_t gc_strideBlockSize_t;
typedef uint_12_t gc_strideBlockNum_t;
typedef uint_16_t gc_stride_t;
typedef gc_payloadSize_t gc_vectorSize_t;
typedef uint_4_t gc_srcVectorNum_t;

// Datamover
typedef uaxis_l<GC_ADDR_WIDTH+17+GC_MAX_PAYLOAD_BYTES> dataMoverCommand_word_t;
typedef hls::stream<dataMoverCommand_word_t> dataMoverCommand_t;
typedef uaxis_l<8> axis_word_8a_t;
typedef hls::stream<axis_word_8a_t> dataMoverStatus_t;
typedef ap_uint<GC_MAX_PAYLOAD_BYTES> btt_t;

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
    axis_dest_t &axis_kernel_out, //output data to kernel
    axis_t &axis_kernel_in //input data from kernel
);

void hold_buffer(
    axis_t &axis_input, //input from GAScore
    axis_t &axis_output, //output AM reply
    uint_1_t &dataRelease
);

#endif // GASCORE_H_
