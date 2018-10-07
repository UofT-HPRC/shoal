#ifndef AM_RX_H_
#define AM_RX_H_

#include "gascore.hpp"
#define DEBUG
#ifdef DEBUG
#include "testbench.hpp"
#include <sstream>
#endif

/* Handler Request Packet Schema

    Short
    | Type (8) | # Arguments (8) | Handler ID (16)  |
    |                   Token                       |
    |           0...16 Handler arguments            |

    Medium
    | Type (8) | # Arguments (8) | Handler ID (16)  |
    |                   Token                       |
    |                   Payload size                |
    |           0...16 Handler arguments            |
    |           Payload                             |

    Long
    | Type (8) | # Arguments (8) | Handler ID (16)  |
    |                   Token                       |
    |                   Payload size                |
    |                   Src address lower           |
    |                   Src address upper           |
    |           0...16 Handler arguments            |

    Long Strided
    | Type (8) | # Arguments (8) | Handler ID (16)  |
    |                   Token                       |
    |                   Payload size                |
    |                   Src address lower           |
    |                   Src address upper           |
    |               Contiguous block size           |
    |               Number of cont. blocks          |
    |               Stride between blocks           |
    |           0...16 Handler arguments            |

    Long Vectored
    | Type (8) | # Arguments (8) | Handler ID (16)  |
    |                   Token                       |
    | Reserved (4) | # dst vectors (4) | payload size (24) |
    |                   Vector 1 size                      |
    |                   Destination (lower)                |
    |                   Destination (upper)                |
    |                   more vectors...                    |
    |                   Handler args                       |

*/

static enum state_t{st_header, st_AMHandlerArgs,
        st_AMLongVector, st_AMdestination, st_AMToken,
        st_AMpayload, st_AMLongStride, st_done, st_error} 
        currentState;

#define DECLARE_VARIABLES\
    axis_t axis_handler("handler");\
    axis_t axis_net("net");\
    dataMoverCommand_t axis_s2mmCommand("s2mmCommand");\
    axis_t axis_s2mm("s2mm");\
    dataMoverStatus_t axis_s2mmStatus("s2mmStatus");\
\
    uint_1_t release;\
\
    axis_word_t axis_word;\
    dataMoverCommand_word_t axis_word_s2mmCommand;\
    axis_word_8a_t axis_word_s2mmStatus;\
\
    uint_64_t readData;\
    uint_1_t readLast;

#ifdef DEBUG
#define CALL_TB am_rx(dbg_currentState, axis_handler, axis_net, \
    axis_s2mmCommand,axis_s2mm, axis_s2mmStatus, release);
#else
#define CALL_TB am_rx(axis_handler, axis_net,axis_s2mmCommand, \
    axis_s2mm, axis_s2mmStatus, release);
#endif

#define WRITE_WORD(Aword, Adata, Alast, Aaxis)\
    Aword.data = Adata;\
    Aword.last = Alast;\
    Aaxis.write(Aword);

#define READ_WORD(Aword, Adata, Alast, Aaxis)\
    Aaxis.read(Aword);\
    Adata = Aword.data;\
    Alast = Aword.last;

//the arg numbers correspond to the c_args order in axis.py
#define WRITE(word_type, interface)\
    WRITE_WORD(word_type, args[0], args[1], interface)

#define READ(word_type, interface)\
    READ_WORD(word_type, readArgs[0], readArgs[1], interface)

#define VERIFY\
    if(args[0] != readArgs[0] || args[1] != readArgs[1]){\
        valid = false;\
        std::cout << "Mismatch at id: " << id << "\n";\
        std::cout << std::hex << "   Expected: " << args[0] << " " << \
            args[1] << "\n";\
        std::cout << std::hex << "   Received: " << readArgs[0] << " " << \
            readArgs[1] << "\n";\
    }

#define PRINT_INTERFACES std::cout << "Stream statuses:\n"; \
    std::cout << std::dec << "  Handler: " << axis_handler.size() << "\n"; \
    std::cout << std::dec << "  Network: " << axis_net.size() << "\n"; \
    std::cout << std::dec << "  s2mmCommand: " << axis_s2mmCommand.size() << "\n"; \
    std::cout << std::dec << "  s2mm: " << axis_s2mm.size() << "\n"; \
    std::cout << std::dec << "  s2mmStatus: " << axis_s2mmStatus.size() << "\n";

#define READ_INTERFACES \
    while(axis_handler.size() > 0){ \
        READ(axis_word, axis_handler) \
        std::cout << std::hex << "Handler - Data: " << axis_word.data << " Last: " << axis_word.last << "\n"; \
    }\
    while(axis_net.size() > 0){ \
        READ(axis_word, axis_net) \
        std::cout << std::hex << "Net - Data: " << axis_word.data << " Last: " << axis_word.last << "\n"; \
    }\
    while(axis_s2mmCommand.size() > 0){ \
        READ(axis_word_s2mmCommand, axis_s2mmCommand) \
        std::cout << std::hex << "s2mmCommand - Data: " << axis_word_s2mmCommand.data << " Last: " << axis_word_s2mmCommand.last << "\n"; \
    }\
    while(axis_s2mm.size() > 0){ \
        READ(axis_word, axis_s2mm) \
        std::cout << std::hex << "s2mm - Data: " << axis_word.data << " Last: " << axis_word.last << "\n"; \
    }\
    while(axis_s2mmStatus.size() > 0){ \
        READ(axis_word_s2mmStatus, axis_s2mmStatus) \
        std::cout << std::hex << "s2mmStatus - Data: " << axis_word_s2mmStatus.data << " Last: " << axis_word_s2mmStatus.last << "\n"; \
    }

void am_rx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_handler, //output
    axis_t &axis_net, //input
    dataMoverCommand_t &axis_s2mmCommand, //output
    axis_t &axis_s2mm, //output
    dataMoverStatus_t &axis_s2mmStatus, //input

    //axis_handler release
    uint_1_t &release //output
);

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif