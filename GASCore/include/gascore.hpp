#ifndef GASCORE_H_
#define GASCORE_H_

// //constant declarations
// #define GC_BIG_ENDIAN 1
// #define GC_M_AXI_ADDR_WIDTH 32
// #define GC_M_AXI_DATA_WIDTH 32
// #define GC_MAX_BURST_LEN 16
// #define GC_MAX_PACKET_BYTES 8192

// //derived constant declarations
// #define GC_DATA_BYTES_PER_WORD (GC_M_AXI_DATA_WIDTH/4)

#include "stream.hpp"
#include "types.hpp"

#define GC_DATA_WIDTH 64UL
#define GC_ADDR_WIDTH 32
#define GC_MAX_PAYLOAD 12 //2^MAX_PAYLOAD words

#define GC_DATA_BYTES (GC_DATA_WIDTH/8)
#define GC_MAX_PAYLOAD_BYTES (GC_MAX_PAYLOAD+NBITS(GC_DATA_BYTES)-1)

typedef uaxis_l<GC_DATA_WIDTH> axis_word_t;
typedef hls::stream<axis_word_t> axis_t;
typedef uint_64_t word_t;
typedef uint_32_t addr_word_t;

/*

Type:
    Bit 0: set if short or long strided
    Bit 1: set if medium or long vectored
    Bit 2: set if a longx message
    Bit 3: reserved
    Bit 4: set if data payload for medium/long message comes from a FIFO
    Bit 5: set if a medium/long message is Async
    Bit 6: set if message is a reply
    Bit 7: reserved
Active Message Packet Schema 
    Short
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    | Reserved (40) | Packet ID (24) |
    |                   Handler args               |
    
    Medium
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    | Reserved (40) | Packet ID (24) |
    |                   Handler args               |
    |                   Payload ...                |

    Long
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    | Reserved (40) | Packet ID (24) |
    |                   Destination                |
    |                   Handler args               |
    |                   Payload ...                |

    Long Stride 
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    | Stride (16) | Cont. block size (12) | blocks (12) | Packet ID (24) |
    |                   Destination                |
    |                   Handler args               |
    |                   Payload ...                |

    Long Vector 
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    | # src vectors (4) | # dst vectors (4) | Reserved (12) | Size 1 (12) | Reserved (8) | Packet ID (24) |
    |                   Destination                |
    | Size 2... (12) | Reserved (52) |
    |                   Destination                |
    ...    
    |                   Handler args               |
    |                   Payload ...                |    

    |0                             32|
    | SRC (8) | DST (8) | Words (16) |
    |               AM               |

    Short AM
    | Type (8) | # Arguments (8) | Handler ID (16) |
    |                   Handler args               |

    Medium AM
    | Type (8) | # Arguments (8) | Handler ID (16) |
    |           Payload size in bytes (32)         |
    |                   Handler args               |
    |                   Payload ...                |

    Long AM
    | Type (8) | # Arguments (8) | Handler ID (16) |
    |           Payload size in bytes (32)         |
    |               Destination (lower)            |
    |               Destination (upper)            |
    |                   Handler args               |
    |                   Payload ...                |

    Long Strided AM
    | Type (8) | # Arguments (8) | Handler ID (16) |
    |           Payload size in bytes (32)         |
    |               Destination (lower)            |
    |               Destination (upper)            |
    |               Contiguous block size          |
    |               Number of cont. blocks         |
    |               Stride between blocks          |
    |                   Handler args               |
    |                   Payload ...                |

    Long Vectored AM
    |  Type (8)  |  # Arguments (8)   |  Handler ID (16)   |
    | Reserved (4) | # dst vectors (4) | payload size (24) |
    |                   Vector 1 size                      |
    |                   Destination (lower)                |
    |                   Destination (upper)                |
    |                   more vectors...                    |
    |                   Handler args                       |
    |                   Payload ...                        |

*/

//Header Types
typedef uint_16_t gc_AMsrc_t; //[7:0] of header
typedef uint_16_t gc_AMdst_t; //[15:8] of header
// typedef uint_12_t gc_AMwords_t; //[31:16] of header

//AM Types
typedef uint_8_t gc_AMtype_t;
typedef uint_8_t gc_AMargs_t;
typedef uint_4_t gc_AMhandler_t;
typedef uint_24_t gc_AMToken_t;

//AM Handler args
typedef uint_64_t gc_AMhandlerArg_t;

//Payload
typedef ap_uint<GC_MAX_PAYLOAD> gc_payloadSize_t;
typedef uint_4_t gc_dstVectorNum_t;

//Long
// typedef uint_32_t gc_destinationLower_t;
// typedef uint_32_t gc_destinationUpper_t;
typedef uint_64_t gc_destination_t;
typedef gc_payloadSize_t gc_strideBlockSize_t;
typedef uint_12_t gc_strideBlockNum_t;
typedef uint_16_t gc_stride_t;
typedef gc_payloadSize_t gc_vectorSize_t;
// typedef uint_32_t gc_vectorDestLower_t;
// typedef uint_32_t gc_vectorDestUpper_t;

// typedef uint_32_t gc_token_t;

typedef uaxis_l<GC_ADDR_WIDTH+17+GC_MAX_PAYLOAD_BYTES> dataMoverCommand_word_t;
typedef hls::stream<dataMoverCommand_word_t> dataMoverCommand_t;

typedef uaxis_l<8> axis_word_8a_t;
typedef hls::stream<axis_word_8a_t> dataMoverStatus_t;

typedef ap_uint<GC_MAX_PAYLOAD_BYTES> btt_t;

#define MAX_VECTOR_NUM 16

bool isShortAM(gc_AMtype_t arg);

bool isMediumAM(gc_AMtype_t arg);

bool isLongxAM(gc_AMtype_t arg);

bool isLongAM(gc_AMtype_t arg);

bool isLongVectoredAM(gc_AMtype_t arg);

bool isLongStridedAM(gc_AMtype_t arg);

bool isFIFOnotMemData(gc_AMtype_t arg);

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

#endif