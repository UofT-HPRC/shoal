#ifndef AM_RX_H_
#define AM_RX_H_

#include "gascore.hpp"
// #define DEBUG
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