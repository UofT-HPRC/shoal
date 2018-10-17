#ifndef XPAMS_H_
#define XPAMS_H_

#include "utilities.hpp"
#define DEBUG
#ifdef DEBUG
#include "testbench.hpp"
#endif

#define H_EMPTY 0

static enum state_t{st_AMheader, st_sendReplyHeader, 
    st_AMpayload} currentState;

#define DECLARE_VARIABLES\
    axis_t axis_rx("rx");\
    axis_t axis_tx("tx");\
    axis_dest_t axis_kernel_out("kernel_out");\
    axis_t axis_kernel_in("kernel_in");\
\
    axis_word_t axis_word;\
    axis_wordDest_t axis_wordDest;\
\
    uint_64_t readData;\
    uint_1_t readLast;

#ifdef DEBUG
#define CALL_TB xpams(dbg_currentState, axis_rx, axis_tx, \
    axis_kernel_out,axis_kernel_in);
#else
#define CALL_TB xpams(axis_rx, axis_tx,axis_kernel_out, \
    axis_kernel_in);
#endif

// #define WRITE_WORD(Aword, Adata, Alast, Aaxis)\
//     Aword.data = Adata;\
//     Aword.last = Alast;\
//     Aaxis.write(Aword);

// #define READ_WORD(Aword, Adata, Alast, Aaxis)\
//     Aaxis.read(Aword);\
//     Adata = Aword.data;\
//     Alast = Aword.last;

// //the arg numbers correspond to the c_args order in axis.py
// #define WRITE(word_type, interface)\
//     WRITE_WORD(word_type, args[0], args[1], interface)

// #define READ(word_type, interface)\
//     READ_WORD(word_type, readArgs[0], readArgs[1], interface)

// #define VERIFY\
//     if(args[0] != readArgs[0] || args[1] != readArgs[1]){\
//         valid = false;\
//         std::cout << "Mismatch at id: " << id << "\n";\
//         std::cout << std::hex << "   Expected: " << args[0] << " " << \
//             args[1] << "\n";\
//         std::cout << std::hex << "   Received: " << readArgs[0] << " " << \
//             readArgs[1] << "\n";\
//     }

#define PRINT_INTERFACES std::cout << "Stream statuses:\n"; \
    PRINT_AXIS_SIZE("RX", axis_rx) \
    PRINT_AXIS_SIZE("TX", axis_tx) \
    PRINT_AXIS_SIZE("Kernel_Out", axis_kernel_out) \
    PRINT_AXIS_SIZE("Kernel_In", axis_kernel_in)
    // std::cout << std::dec << "  RX: " << axis_rx.size() << "\n"; \
    // std::cout << std::dec << "  TX: " << axis_tx.size() << "\n"; \
    // std::cout << std::dec << "  Kernel_Out: " << axis_kernel_out.size() << "\n"; \
    // std::cout << std::dec << "  Kernel_In: " << axis_kernel_in.size() << "\n";

#define READ_INTERFACES \
    READ_STREAM_INTERFACE("RX", uaxis_l, axis_rx, axis_word)\
    READ_STREAM_INTERFACE("TX", uaxis_l, axis_tx, axis_word)\
    READ_STREAM_INTERFACE("Kernel_Out", uaxis_m, axis_kernel_out, axis_wordDest)\
    READ_STREAM_INTERFACE("Kernel_In", uaxis_l, axis_kernel_in, axis_word)
    // while(axis_rx.size() > 0){ \
    //     READ(axis_word, axis_rx) \
    //     std::cout << std::hex << "RX - Data: " << axis_word.data << " Last: " << axis_word.last << "\n"; \
    // }\
    // while(axis_tx.size() > 0){ \
    //     READ(axis_word, axis_tx) \
    //     std::cout << std::hex << "TX - Data: " << axis_word.data << " Last: " << axis_word.last << "\n"; \
    // }\
    // while(axis_kernel_out.size() > 0){ \
    //     READ(axis_word, axis_kernel_out) \
    //     std::cout << std::hex << "Kernel_out - Data: " << axis_word.data << " Last: " << axis_word.last << "\n"; \
    // }\
    // while(axis_kernel_in.size() > 0){ \
    //     READ(axis_word, axis_kernel_in) \
    //     std::cout << std::hex << "Kernel_in - Data: " << axis_word.data << " Last: " << axis_word.last << "\n"; \
    }

void xpams(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_rx, //input from GASCore
    axis_t &axis_tx, //output AM reply
    axis_dest_t &axis_kernel_out, //output data to kernel
    axis_t &axis_kernel_in //input data from kernel
);

inline axis_wordDest_t assignWord(axis_word_t axis_word);

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif