#ifndef TYPES_H_
#define TYPES_H_

//needed for a stupid HLS bug where it searches the system headers before the 
//Vivado ones during cosim. This is apparently a known issue in at least 2017.2 
//for designs that use fixed point.
// #include "/media/sharm294/HDD_1TB/Xilinx/Vivado_HLS/2017.2/include/gmp.h"

#include "ap_int.h"

//typedefs

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

#endif
