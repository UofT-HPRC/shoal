#ifndef AM_RX_H_
#define AM_RX_H_

#include "globals.hpp"
#include "packet_defs.hpp"

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

typedef uaxis_l<72> axis_word_72a_t;

typedef hls::stream<axis_word_72a_t> s2mmCommand_t;

void am_rx(
    axis_32a_t &axis_handler, //output
    axis_32a_t &axis_net, //input
    s2mmCommand_t &axis_s2mmCommand, //output
    axis_32a_t &axis_s2mm, //output
    axis_32a_t &axis_s2mmStatus, //input

    //token RAM
    uint_16_t token_get, //input
    uint_1_t token_get_v, //input
    uint_8_t &record, //output

    //axis_handler release
    uint_1_t &release, //output

    int &dbg_state
);

void s2mmWriteCommand(
    s2mmCommand_t &axis_s2mmCommand, //output
    uint_4_t reserved,
    uint_4_t tag,
    uint_32_t address,
    uint_1_t ddr,
    uint_1_t eof,
    uint_6_t dsa,
    uint_1_t type,
    uint_23_t btt
);

#endif
