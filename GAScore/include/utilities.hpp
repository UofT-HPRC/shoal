#ifndef UTILITIES_H_
#define UTILITIES_H_

/*******************************************************************************
 * Includes
*******************************************************************************/

#ifndef __HLS__
#define __HLS__
#endif
#define USE_APUINT
// #include "active_messages.hpp"
#include "hls_types.hpp"
#include "stream.hpp"
#include "global_utilities.hpp"

// Datamover
typedef uaxis_o<GC_ADDR_WIDTH+40> dataMoverCommand_word_t;
typedef hls::stream<dataMoverCommand_word_t> dataMoverCommand_t;
typedef uaxis_l<8> axis_word_8a_t;
typedef hls::stream<axis_word_8a_t> dataMoverStatus_t;
typedef ap_uint<23> btt_t;

/*******************************************************************************
 * Macros
*******************************************************************************/

#define COMPARE_uaxis_l(x) x
#define COMPARE_uaxis_m(x) x
#define COMPARE_uaxis_n(x) x

#define WRITE_WORD_L(Aword, Adata, Alast, Akeep, Aaxis)\
    Aword.data = Adata;\
    Aword.last = Alast;\
    Aword.keep = Akeep;\
    Aaxis.write(Aword);

#define WRITE_WORD_M(Aword, Adata, Alast, Akeep, Adest, Aaxis)\
    Aword.data = Adata;\
    Aword.last = Alast;\
    Aword.keep = Akeep;\
    Aword.dest = Adest;\
    Aaxis.write(Aword);

#define WRITE_WORD_N(Aword, Adata, Alast, Aaxis)\
    Aword.data = Adata;\
    Aword.last = Alast;\
    Aaxis.write(Aword);

#define READ_WORD_L(Aword, Adata, Alast, Akeep, Aaxis)\
    Aaxis.read(Aword);\
    Adata = Aword.data;\
    Alast = Aword.last;\
    Akeep = Aword.keep;

#define READ_WORD_M(Aword, Adata, Alast, Akeep, Adest, Aaxis)\
    Aaxis.read(Aword);\
    Adata = Aword.data;\
    Alast = Aword.last;\
    Akeep = Aword.keep;\
    Adest = Aword.dest;

#define READ_WORD_N(Aword, Adata, Alast, Aaxis)\
    Aaxis.read(Aword);\
    Adata = Aword.data;\
    Alast = Aword.last;

//the arg numbers correspond to the c_args order in axis.py in Sonar
#define WRITE(key, word_type, interface)\
    WHEN(EQUAL(key, uaxis_l))\
        (WRITE_WORD_L(word_type, args[0], args[1], args[2], interface))\
    WHEN(EQUAL(key, uaxis_m))\
        (WRITE_WORD_M(word_type, args[0], args[1], args[2], args[3], interface))\
    WHEN(EQUAL(key, uaxis_n))\
        (WRITE_WORD_N(word_type, args[0], args[1], interface))

#define READ(key, word_type, interface)\
    WHEN(EQUAL(key, uaxis_l))\
        (READ_WORD_L(word_type, readArgs[0], readArgs[1], readArgs[2], interface))\
    WHEN(EQUAL(key, uaxis_m))\
        (READ_WORD_M(word_type, readArgs[0], readArgs[1], readArgs[2], readArgs[3], interface))\
    WHEN(EQUAL(key, uaxis_n))\
        (READ_WORD_N(word_type, readArgs[0], readArgs[1], interface))

#define VERIFY(key)\
    if(!strcmp(key,"uaxis_l")){\
        if(args[0] != readArgs[0] || args[1] != readArgs[1] || \
        args[2] != readArgs[2]){\
            valid = false;\
            std::cout << "Mismatch at id: " << id << "\n";\
            std::cout << std::hex << "   Expected: " << args[0] << " " << \
                args[1] << " " << args[2] << "\n";\
            std::cout << std::hex << "   Received: " << readArgs[0] << " " << \
                readArgs[1] << " " << readArgs[2] << "\n";\
        }\
    }\
    else if(!strcmp(key,"uaxis_m")){\
        if(args[0] != readArgs[0] || args[1] != readArgs[1] || \
        args[2] != readArgs[2] || args[3] != readArgs[3]){\
            valid = false;\
            std::cout << "Mismatch at id: " << id << "\n";\
            std::cout << std::hex << "   Expected: " << args[0] << " " << \
                args[1] << " " << args[2] << " " << args[3] << "\n";\
            std::cout << std::hex << "   Received: " << readArgs[0] << " " << \
                readArgs[1] << " " << readArgs[2] << " " << readArgs[3] << "\n";\
        }\
    }\
    if(!strcmp(key,"uaxis_n")){\
        if(args[0] != readArgs[0] || args[1] != readArgs[1]){\
            valid = false;\
            std::cout << "Mismatch at id: " << id << "\n";\
            std::cout << std::hex << "   Expected: " << args[0] << " " << \
                args[1] << "\n";\
            std::cout << std::hex << "   Received: " << readArgs[0] << " " << \
                readArgs[1] << "\n";\
        }\
    }\

#define READ_STREAM_INTERFACE(name, key, stream, word_type) \
    while(stream.size() > 0){ \
        WHEN(EQUAL(key, uaxis_l))(\
            READ(key, word_type, stream) \
            std::cout << std::hex << name << " - Data: " << word_type.data << \
            " Last: " << word_type.last << " keep: " << word_type.keep << "\n"; \
        )\
        WHEN(EQUAL(key, uaxis_m))(\
            READ(key, word_type, stream) \
            std::cout << std::hex << name << " - Data: " << word_type.data << \
            " Last: " << word_type.last << " Keep: " << word_type.keep << \
            " Dest: " << word_type.dest << "\n"; \
        )\
        WHEN(EQUAL(key, uaxis_n))(\
            READ(key, word_type, stream) \
            std::cout << std::hex << name << " - Data: " << word_type.data << \
            " Last: " << word_type.last << "\n"; \
        )\
    }

/*******************************************************************************
 * Function Prototypes
*******************************************************************************/

void dataMoverWriteCommand(
    dataMoverCommand_t &axis_command, //output
    uint_4_t reserved,
    uint_4_t tag,
    addr_word_t address,
    uint_1_t ddr,
    uint_1_t eof,
    uint_6_t dsa,
    uint_1_t type,
    btt_t btt
);

inline axis_wordNoKeep_t assignWordtoNoKeep(axis_word_t axis_word){
    axis_wordNoKeep_t axis_wordDest;

    axis_wordDest.data = axis_word.data;
    axis_wordDest.last = axis_word.last;
    
    return axis_wordDest;
}

inline axis_wordDest_t assignWord(axis_word_t axis_word){
    axis_wordDest_t axis_wordDest;

    axis_wordDest.data = axis_word.data;
    axis_wordDest.keep = axis_word.keep;
    axis_wordDest.last = axis_word.last;
    
    return axis_wordDest;
}

#endif
