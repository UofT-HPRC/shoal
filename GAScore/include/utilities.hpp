#ifndef UTILITIES_H_
#define UTILITIES_H_

/*******************************************************************************
 * Includes
*******************************************************************************/

#include "../../include/config.hpp"
#include "shoal_stream.hpp"
#include "shoal_utilities.hpp"

/*******************************************************************************
 * Defines
*******************************************************************************/

#define GC_DATA_WIDTH 64UL // bit width of tdata
#define GC_ADDR_WIDTH 32 // bit width of addressable memory on a node
#define GC_DEST_WIDTH 16 // bit width of tdest
#define MAX_VECTOR_NUM 16 // Maximum number of vectors for vectored messages

/*******************************************************************************
 * Derived defines
*******************************************************************************/

#define GC_MAX_PAYLOAD (AM_PAYLOAD_SIZE_UPPER - AM_PAYLOAD_SIZE_LOWER + 1)
#define GC_DATA_BYTES (GC_DATA_WIDTH/8)
#define GC_MAX_PAYLOAD_BYTES (GC_MAX_PAYLOAD+NBITS(GC_DATA_BYTES)-1)
#define GC_DATA_TKEEP (power<2, GC_DATA_BYTES>()-1)

/*******************************************************************************
 * Typedefs
*******************************************************************************/

typedef uaxis_l<GC_DATA_WIDTH> axis_word_t;
typedef hls::stream<axis_word_t> axis_t;
typedef uaxis_m<GC_DATA_WIDTH, GC_DEST_WIDTH> axis_wordDest_t;
typedef hls::stream<axis_wordDest_t> axis_dest_t;
typedef uaxis_n<GC_DATA_WIDTH> axis_wordNoKeep_t;
typedef hls::stream<axis_wordNoKeep_t> axis_noKeep_t;

typedef ap_uint<GC_DATA_WIDTH> word_t;
typedef ap_uint<GC_ADDR_WIDTH> addr_word_t;

typedef ap_uint<GC_DEST_WIDTH> gc_AMdest_t;

// Header Types
typedef ap_uint<16> gc_AMsrc_t;
typedef uint_16_t gc_AMdst_t;

// AM Types
typedef uint_8_t gc_AMtype_t;
typedef uint_8_t gc_AMargs_t;
typedef uint_4_t gc_AMhandler_t;
typedef uint_24_t gc_AMToken_t;

// Payload
typedef ap_uint<GC_MAX_PAYLOAD> gc_payloadSize_t;
typedef uint_4_t gc_dstVectorNum_t;

// Long
typedef uint_64_t gc_destination_t;
typedef gc_payloadSize_t gc_strideBlockSize_t;
typedef uint_12_t gc_strideBlockNum_t;
typedef uint_16_t gc_stride_t;
typedef gc_payloadSize_t gc_vectorSize_t;
typedef uint_4_t gc_srcVectorNum_t;

// Datamover
//TODO change uaxis_l to uaxis_o for the command
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

bool isShortAM(gc_AMtype_t arg);

bool isMediumAM(gc_AMtype_t arg);

bool isLongxAM(gc_AMtype_t arg);

bool isLongAM(gc_AMtype_t arg);

bool isLongVectoredAM(gc_AMtype_t arg);

bool isLongStridedAM(gc_AMtype_t arg);

bool isDataFromFIFO(gc_AMtype_t arg);

bool isAsyncAM(gc_AMtype_t arg);

bool isReplyAM(gc_AMtype_t arg);

bool isMediumFIFOAM(gc_AMtype_t arg);

bool isLongFIFOAM(gc_AMtype_t arg);

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
