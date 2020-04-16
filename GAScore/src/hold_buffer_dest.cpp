#include "hold_buffer_dest.hpp"

/**
 * TODO there's a two cycle latency from input to output for some reason.
 * TODO ^Some shift registers are being inferred.
*/


// count_t read(
//     axis_t &axis_input,
//     axis_t &axis_fifo,
//     uint_1_t &dataRelease
// ){
//     #pragma HLS PIPELINE II=1

//     axis_word_t axis_word;
//     static count_t writeCount = 0;
//     static count_t holdCount = 0;
//     #pragma HLS RESET variable=writeCount
//     #pragma HLS RESET variable=holdCount
//     // count_t holdCount;
//     // uint_1_t writing;

//     if (!axis_input.empty()/* && !axis_fifo.full()*/){
//         axis_input.read(axis_word);
//         axis_fifo.write(axis_word);
//         writeCount++;
//         // writing = 1;
//     }
//     // else
//     //     writing = 0;

//     if (dataRelease){
//         // if (writing)
//         //     holdCount = writeCount;
//         // else
//             holdCount = writeCount;
//     }

//     return holdCount;
// }

// void write(
//     axis_t &axis_output,
//     axis_t &axis_fifo,
//     count_t &holdCount
// ){
//     #pragma HLS DATAFLOW

//     axis_word_t axis_word;
//     static count_t readCount = 0;
//     #pragma HLS RESET variable=readCount

//     #define MSB_2 LOG_FIFO_DEPTH-1,LOG_FIFO_DEPTH-3
//     if (!axis_fifo.empty() /*&& !axis_output.full()*/){
//         if(holdCount > readCount || (holdCount(MSB_2) == 0 && readCount(MSB_2) == 3)){
//             axis_fifo.read(axis_word);
//             axis_output.write(axis_word);
//             readCount++;
//         }
//     }
//     #undef MSB_2
// }

// void hold_buffer(
//     axis_t &axis_input,
//     axis_t &axis_output,
//     uint_1_t &dataRelease
// ){
//     #pragma HLS INTERFACE axis port=axis_input
//     #pragma HLS INTERFACE axis port=axis_output
//     #pragma HLS INTERFACE ap_stable port=dataRelease
// 	#pragma HLS INTERFACE ap_ctrl_none port=return

//     count_t holdCount;
//     uint_1_t writing;

//     axis_t axis_fifo("fifo");
//     ADD_PRAGMA(HLS STREAM depth=FIFO_DEPTH variable=axis_fifo)

//     #pragma HLS DATAFLOW

//     holdCount = read(axis_input, axis_fifo, dataRelease);
//     write(axis_output, axis_fifo, holdCount);
// }

static void read(axis_dest_t &axis_input, axis_dest_t &axis_output){
    #pragma HLS PIPELINE
    axis_wordDest_t axis_word;
    axis_input.read(axis_word);
    axis_output.write(axis_word);
}

static void write(axis_dest_t &axis_input, axis_dest_t &axis_output, uint_1_t &dataRelease){
    #pragma HLS PIPELINE
    axis_wordDest_t axis_word;
    if(dataRelease){
        axis_input.read(axis_word);
        axis_output.write(axis_word);
    }
}

void hold_buffer_dest(axis_dest_t &axis_input, axis_dest_t &axis_output, uint_1_t &dataRelease){
    #pragma HLS INTERFACE axis port=axis_input
    #pragma HLS INTERFACE axis port=axis_output
    #pragma HLS INTERFACE ap_stable port=dataRelease
    #pragma HLS INTERFACE ap_ctrl_none port=return
    // #pragma HLS DATAFLOW

    // static axis_dest_t temp;
    // ADD_PRAGMA(HLS STREAM depth=FIFO_DEPTH variable=temp dim=1)
    // #pragma HLS data_pack variable=temp

    // read(axis_input, temp);
    // write(temp, axis_output, dataRelease);

    #pragma HLS PIPELINE
    // axis_wordDest_t axis_word;
    // axis_word = axis_input.read();
    axis_output.write(axis_input.read());
}
