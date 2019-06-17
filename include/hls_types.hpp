#ifndef SHOAL_INCLUDE_HLS_TYPES_H_
#define SHOAL_INCLUDE_HLS_TYPES_H_

/**
 * needed for a stupid HLS bug where it searches the system headers before the 
 * Vivado ones during cosim. This is apparently a known issue in at least 2017.2 
 * for designs that use fixed point.
 **/
// #include "/media/sharm294/HDD_1TB/Xilinx/Vivado_HLS/2017.2/include/gmp.h"

#include "ap_int.h"
#include "stream.hpp"
#include "global_utilities.hpp" // for power

//unsigned int
typedef ap_uint<1> uint_1_t;
typedef ap_uint<2> uint_2_t;
typedef ap_uint<3> uint_3_t;
typedef ap_uint<4> uint_4_t;
typedef ap_uint<6> uint_6_t;
typedef ap_uint<7> uint_7_t;
typedef ap_uint<8> uint_8_t;
typedef ap_uint<9> uint_9_t;
typedef ap_uint<11> uint_11_t;
typedef ap_uint<12> uint_12_t;
typedef ap_uint<13> uint_13_t;
typedef ap_uint<16> uint_16_t;
typedef ap_uint<22> uint_22_t;
typedef ap_uint<23> uint_23_t;
typedef ap_uint<24> uint_24_t;
typedef ap_uint<32> uint_32_t;
typedef ap_uint<64> uint_64_t;
typedef ap_uint<72> uint_72_t;

/*******************************************************************************
 * Defines
*******************************************************************************/

#define GC_DATA_WIDTH 64ULL // bit width of tdata
#define GC_ADDR_WIDTH 32 // bit width of addressable memory on a node
#define GC_DEST_WIDTH 16 // bit width of tdest
#define MAX_VECTOR_NUM 16 // Maximum number of vectors for vectored messages

/*******************************************************************************
 * Derived defines
*******************************************************************************/

#define GC_MAX_PAYLOAD (AM_PAYLOAD_SIZE_UPPER - AM_PAYLOAD_SIZE_LOWER + 1)
#define GC_DATA_BYTES (GC_DATA_WIDTH/8)
#define GC_DATA_TKEEP (POWER_2(GC_DATA_BYTES)-1)

/*******************************************************************************
 * Typedefs
*******************************************************************************/

typedef uaxis_l<GC_DATA_WIDTH> axis_word_t;
typedef hls::stream<axis_word_t> axis_t;
typedef uaxis_m<GC_DATA_WIDTH, GC_DEST_WIDTH> axis_wordDest_t;
typedef hls::stream<axis_wordDest_t> axis_dest_t;
typedef uaxis_n<GC_DATA_WIDTH> axis_wordNoKeep_t;
typedef hls::stream<axis_wordNoKeep_t> axis_noKeep_t;

#ifdef USE_APUINT

typedef ap_uint<GC_DATA_WIDTH> word_t;
typedef ap_uint<GC_ADDR_WIDTH> addr_word_t;

typedef ap_uint<GC_DEST_WIDTH> gc_AMdest_t;
typedef ap_uint<GC_DATA_TKEEP> gc_keep_t;

// Header Types
typedef uint_16_t gc_AMsrc_t;
typedef uint_16_t gc_AMdst_t;

// AM Types
typedef uint_8_t gc_AMtype_t;
typedef uint_8_t gc_AMargs_t;
typedef uint_4_t gc_AMhandler_t;
typedef uint_24_t gc_AMToken_t;

// Payload
typedef ap_uint<GC_MAX_PAYLOAD> gc_payloadSize_t;
typedef uint_4_t gc_dstVectorNum_t;

// Long
typedef uint_64_t gc_destination_t;
typedef gc_payloadSize_t gc_strideBlockSize_t;
typedef uint_12_t gc_strideBlockNum_t;
typedef uint_12_t gc_stride_t;
typedef gc_payloadSize_t gc_vectorSize_t;
typedef uint_4_t gc_srcVectorNum_t;

#else

typedef long long word_t;
typedef int addr_word_t;

typedef short gc_AMdest_t;
typedef short gc_keep_t;

// Header Types
typedef short gc_AMsrc_t;
typedef short gc_AMdst_t;

// AM Types
typedef short gc_AMtype_t;
typedef short gc_AMargs_t;
typedef short gc_AMhandler_t;
typedef int gc_AMToken_t;

// Payload
typedef short gc_payloadSize_t;
typedef short gc_dstVectorNum_t;

// Long
typedef long long gc_destination_t;
typedef gc_payloadSize_t gc_strideBlockSize_t;
typedef short gc_strideBlockNum_t;
typedef short gc_stride_t;
typedef gc_payloadSize_t gc_vectorSize_t;
typedef short gc_srcVectorNum_t;

#endif

#endif // SHOAL_INCLUDE_HLS_TYPES_H_
