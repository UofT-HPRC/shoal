#ifndef SHOAL_INCLUDE_STREAM_H_
#define SHOAL_INCLUDE_STREAM_H_

// #include "hls_types.hpp"
// #include "global_utilities.hpp"
// #include "ap_axi_sdata.h"
#include "hls_stream.h"

template<int D>
struct uaxis_l{
    ap_uint<D> data;
    ap_uint<D/8> keep;
    ap_uint<1> last;
};

template<int D, int E>
struct uaxis_m{
    ap_uint<D> data;
    ap_uint<D/8> keep;
    ap_uint<1> last;
    ap_uint<E> dest;
};

template<int D>
struct uaxis_n{
    ap_uint<D> data;
    ap_uint<1> last;
};

template<int D>
struct uaxis_o{
    ap_uint<D> data;
};

template<int D, int E, int F, int G>
struct uaxis_p{
    ap_uint<D> data;
    ap_uint<D/8> keep;
    ap_uint<1> last;
    ap_uint<E> dest;
    ap_uint<F> user;
    ap_uint<G> id;
};

#define PRINT_AXIS_SIZE(key, stream) \
    std::cout << std::dec << "  " << key << ": " << stream.size() << "\n";

#endif // SHOAL_INCLUDE_STREAM_H_
