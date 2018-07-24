#ifndef GLOBALS_H_
#define GLOBALS_H_


#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "hls_stream.h"

//typedefs

//int
typedef ap_uint<1> uint_1_t;
typedef ap_uint<4> uint_4_t;
typedef ap_uint<6> uint_6_t;
typedef ap_uint<7> uint_7_t;
typedef ap_uint<8> uint_8_t;
typedef ap_uint<9> uint_9_t;
typedef ap_uint<12> uint_12_t;
typedef ap_uint<13> uint_13_t;
typedef ap_uint<16> uint_16_t;
typedef ap_uint<22> uint_22_t;
typedef ap_uint<23> uint_23_t;
typedef ap_uint<32> uint_32_t;
typedef ap_uint<64> uint_64_t;
typedef ap_uint<72> uint_72_t;

template<int D>
struct uaxis_l{
    ap_uint<D> data;
    ap_uint<1> last;
};

//stream - DATA, USER, ID, DEST bits, in that order
typedef uaxis_l<32> axis_word_32a_t;

typedef hls::stream<axis_word_32a_t> axis_32a_t;

#define WRITE_WORD(Aword, Adata, Alast, Aaxis)\
    Aword.data = Adata;\
    Aword.last = Alast;\
    Aaxis.write(Aword);

#define READ_WORD(Aword, Adata, Alast, Aaxis)\
    Aaxis.read(Aword);\
    Adata = Aword.data;\
    Alast = Aword.last;

#endif
