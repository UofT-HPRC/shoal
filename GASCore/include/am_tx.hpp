#ifndef AM_TX_H_
#define AM_TX_H_

#include "gascore.hpp"
#define DEBUG
#ifdef DEBUG
#include "testbench.hpp"
#include <sstream>
#endif

/* AM Request Packet Schema

    Short
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    |                   Token                      |
    |                   Handler args               |
    
    Medium (FIFO 0)
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    |                   Token                      |
    |                   SRC addr                   |
    |                   Handler args               |

    Medium (FIFO 1)
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    |                   Token                      |
    |                   Handler args               |
    |                   Payload ...                |

    Long (FIFO 0)
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    |                   Token                      |
    |                   Source Addr                |
    |                   Destination                |
    |                   Handler args               |

    Long (FIFO 1)
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    |                   Token                      |
    |                   Destination                |
    |                   Handler args               |
    |                   Payload ...                |

    Long Stride 
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    | Stride (16) | Cont. block size (12) | blocks (12) | Reserved (24) |
    |                   Source addr                |
    | Stride (16) | Cont. block size (12) | blocks (12) | Packet ID (24) |
    |                   Dest.  addr                |
    |                   Handler args               |

    Long Vector 
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    | # src vectors (4) | # dst vectors (4) | src Size 1 (12) | dst Size 1 (12) | Reserved (8) | Packet ID (24) |
    |                   src   addr.                |
    |                   dst   addr.                |
    | src size 2... (12) | Reserved (52) |
    |                   src   addr.                |
    ...
    | dst size 2... (12) | Reserved (52) |
    |                   dst   addr.                |
    ...    
    |                   Handler args               |

    Short
    | Type (8) | # Arguments (8) | Token (16)  |
    |       Handler ID (16)      | Source (16) |
    |           0...16 Handler arguments       |

    Medium (FIFO 0)
    | Type (8) | # Arguments (8) | Token (16)  |
    |       Handler ID (16)      | Source (16) |
    |           Payload size                   |
    |           Src address lower              |
    |           Src address upper              |
    |           0...16 Handler arguments       |

    Medium (FIFO 1)
    | Type (8) | # Arguments (8) | Token (16)  |
    |       Handler ID (16)      | Source (16) |
    |           Payload size                   |
    |           0...16 Handler arguments       |
    |           0...512 words payload          |

    Long (FIFO 0)
    | Type (8) | # Arguments (8) | Token (16)  |
    |       Handler ID (16)      | Source (16) |
    |           Payload size                   |
    |           Src address lower              |
    |           Src address upper              |
    |           dst address lower              |
    |           dst address upper              |
    |           0...16 Handler arguments       |

    Long (FIFO 1)
    | Type (8) | # Arguments (8) | Token (16)  |
    |       Handler ID (16)      | Source (16) |
    |           Payload size                   |
    |           dst address lower              |
    |           dst address upper              |
    |           0...16 Handler arguments       |
    |           0...512 words payload          |

    Long Strided
    | Type (8) | # Arguments (8) | Token (16)  |
    |       Handler ID (16)      | Source (16) |
    |           Payload size                   |
    |           Src address lower              |
    |           Src address upper              |
    |           Src stride in bytes            |
    |           Src block size                 |
    |           Src block number               |
    |           Dst address lower              |
    |           Dst address upper              |
    |           Dst stride in bytes            |
    |           Dst block size                 |
    |           Dst block number               |
    |           0...16 Handler arguments       |

    Long Vectored
    | Type (8) | # Arguments (8) | Token (16)  |
    |       Handler ID (16)      | Source (16) |
    | src vectors (4) | dst vectors (4) | payload size (24) |
    |           Src vector 1 size              |
    |           Src address (lower)            |
    |           Srd address (upper)            |
    |           more vectors...                |
    |           Dst vector 1 size              |
    |           Dst address (lower)            |
    |           Dst address (upper)            |
    |           more vectors...                |
    |           0...16 Handler arguments       |

*/

typedef uint_4_t gc_srcVectorNum_t;

enum state_t{st_header, st_AMHandlerArgs,
    st_AMLongVector, st_AMdestination, st_AMToken,
    st_AMpayload, st_AMLongStride, st_done, st_error, st_AMsource};

void am_tx(
    #ifdef DEBUG
    int &dbg_currentState,
    #endif
    axis_t &axis_kernel, //input
    axis_t &axis_net, //output
    dataMoverCommand_t &axis_mm2sCommand, //output
    axis_t &axis_mm2s, //input
    dataMoverStatus_t &axis_mm2sStatus, //input
    uint_1_t &release //output
);

#ifdef DEBUG
std::string stateParse(int state);
#endif

#endif