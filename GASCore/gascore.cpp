#include "gascore.hpp"

void gascore(
    int* mem
){
    #pragma HLS INTERFACE m_axi port=mem depth=
	#pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS
}