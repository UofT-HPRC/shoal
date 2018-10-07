#ifndef STREAM_H_
#define STREAM_H_

#include "types.hpp"
#include "ap_axi_sdata.h"
#include "hls_stream.h"

template<int D>
struct uaxis_l{
    ap_uint<D> data;
    ap_uint<1> last;
};

//stream - DATA, USER, ID, DEST bits, in that order
// typedef uaxis_l<64> axis_word_64a_t;
// typedef uaxis_l<8> axis_word_8a_t;

// typedef hls::stream<axis_word_64a_t> axis_64a_t;
// typedef hls::stream<axis_word_8a_t> axis_8a_t;

// #define WRITE_WORD(Aword, Adata, Alast, Aaxis)\
//     Aword.data = Adata;\
//     Aword.last = Alast;\
//     Aaxis.write(Aword);

// #define READ_WORD(Aword, Adata, Alast, Aaxis)\
//     Aaxis.read(Aword);\
//     Adata = Aword.data;\
//     Alast = Aword.last;

#endif