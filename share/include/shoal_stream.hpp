#ifndef STREAM_H_
#define STREAM_H_

#include "shoal_types.hpp"
#include "ap_axi_sdata.h"
#include "hls_stream.h"

template<int D>
struct uaxis_l{
    ap_uint<D> data;
    ap_uint<NBITS(D)+1> keep;
    ap_uint<1> last;
};

template<int D, int E>
struct uaxis_m{
    ap_uint<D> data;
    ap_uint<NBITS(D)+1> keep;
    ap_uint<1> last;
    ap_uint<E> dest;
};

#define PRINT_AXIS_SIZE(key, stream) \
    std::cout << std::dec << "  " << key << ": " << stream.size() << "\n";

#endif