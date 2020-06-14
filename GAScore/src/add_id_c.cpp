#include "utilities.hpp"

#define MAX_SIZE 16384

void g2N_ingress(
    axis_dest_t* axis_in,
    ap_uint<8*GC_DATA_BYTES> buffer_storage[MAX_SIZE/GC_DATA_BYTES],
    hls::stream< ap_uint<GALAPAGOS_USER_WIDTH> > &length_stream,
    // hls::stream< ap_uint<GC_DEST_WIDTH> > &dest_stream, 
    hls::stream< ap_uint<GALAPAGOS_ID_WIDTH> > &src_stream 
){
    axis_wordDest_t axis_word;
    
    axis_word = axis_in->read();
    ap_uint<GALAPAGOS_ID_WIDTH> id = axis_word.data(39,24);
    buffer_storage[0] = axis_word.data;

    for(int i=1; i<MAX_SIZE/GC_DATA_BYTES; i++){
        #pragma HLS pipeline II=1
        axis_word = axis_in->read();
        buffer_storage[i] = axis_word.data;
        if(axis_word.last){
            length_stream.write(i+1);
            // dest_stream.write(axis_word.dest);
            src_stream.write(id);
            break;
        }
    }
}


void g2N_egress(
    axis_galapagos_t* axis_out,
    ap_uint<8*GC_DATA_BYTES>buffer_storage[MAX_SIZE/GC_DATA_BYTES],
    hls::stream< ap_uint<GALAPAGOS_USER_WIDTH> > &length_stream,
    // hls::stream< ap_uint<GC_DEST_WIDTH> > &dest_stream, 
    hls::stream< ap_uint<GALAPAGOS_ID_WIDTH> > &src_stream 
){

    ap_uint<GALAPAGOS_USER_WIDTH> length = length_stream.read();
    // ap_uint<GC_DEST_WIDTH> dest = dest_stream.read();
    ap_uint<GALAPAGOS_ID_WIDTH> src = src_stream.read();
    
    axis_wordGalapagos_t axis_word;

    // axis_out->write(axis_word);
    // axis_word.keep = 0xff;
    // axis_word.last = 1;

    for(int i=0; i<(length);i++){
        #pragma HLS pipeline II=1
        axis_word.data = buffer_storage[i];
        axis_word.user = length;
        axis_word.id = src;
        axis_word.last = (i == (length-1));
        axis_out->write(axis_word);
    }

}

void add_id_c(
    axis_dest_t* axis_in,
    axis_galapagos_t* axis_out,
    word_t buffer_storage_A[MAX_SIZE/GC_DATA_BYTES],
    word_t buffer_storage_B[MAX_SIZE/GC_DATA_BYTES]
){

    #pragma HLS dataflow

    #pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE axis port=axis_in
    #pragma HLS INTERFACE axis port=axis_out

    #pragma HLS interface port=buffer_storage_A bram
    #pragma HLS RESOURCE variable=buffer_storage_A core=RAM_1P
    #pragma HLS interface port=buffer_storage_B bram
    #pragma HLS RESOURCE variable=buffer_storage_B core=RAM_1P

    hls::stream< ap_uint<GALAPAGOS_USER_WIDTH> > length_stream;
    // hls::stream< ap_uint<GC_DEST_WIDTH> > dest_stream;
    hls::stream< ap_uint<GALAPAGOS_ID_WIDTH> > src_stream;

    #pragma HLS stream variable=length_stream depth=256
    // #pragma HLS stream variable=dest_stream depth=256
    #pragma HLS stream variable=src_stream depth=256


    g2N_ingress(
        axis_in,
        buffer_storage_A,
        length_stream,
        // dest_stream, 
        src_stream 
    );
    
    g2N_egress(
        axis_out,
        buffer_storage_B,
        length_stream,
        // dest_stream, 
        src_stream 
    );

}