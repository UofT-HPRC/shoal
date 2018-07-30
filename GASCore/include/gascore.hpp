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

#define GC_DATA_WIDTH 32
#define GC_ADDR_WIDTH 32

typedef uaxis_l<GC_DATA_WIDTH> axis_word_t;
typedef hls::stream<axis_word_t> axis_t;

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
typedef uint_8_t gc_AMsrc_t; //[7:0] of header
typedef uint_8_t gc_AMdst_t; //[15:8] of header
typedef uint_16_t gc_AMwords_t; //[31:16] of header

//AM Types
typedef uint_8_t gc_AMtype_t;
typedef uint_8_t gc_AMargs_t;
typedef uint_16_t gc_AMhandler_t;

//AM Handler args
typedef uint_32_t gc_AMhandlerArg_t;

//Payload
typedef uint_32_t gc_payloadSize_t;
typedef uint_4_t gc_dstVectorNum_t;

//Long
typedef uint_32_t gc_destinationLower_t;
typedef uint_32_t gc_destinationUpper_t;
typedef uint_32_t gc_strideBlockSize_t;
typedef uint_32_t gc_strideBlockNum_t;
typedef uint_32_t gc_stride_t;
typedef uint_32_t gc_vectorSize_t;
typedef uint_32_t gc_vectorDestLower_t;
typedef uint_32_t gc_vectorDestUpper_t;

typedef uint_32_t gc_token_t;

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

#endif