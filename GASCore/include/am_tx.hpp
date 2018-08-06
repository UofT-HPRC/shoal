#ifndef AM_TX_H_
#define AM_TX_H_

#include "gascore.hpp"

/* AM Request Packet Schema

    Short
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    |                   Handler args               |
    
    Medium (FIFO 0)
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    |                   SRC addr                   |
    |                   Handler args               |

    Medium (FIFO 1)
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    |                   Handler args               |
    |                   Payload ...                |

    Long (FIFO 0)
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    |                   Source Addr                |
    |                   Destination                |
    |                   Handler args               |

    Long (FIFO 1)
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    |                   Destination                |
    |                   Handler args               |
    |                   Payload ...                |

    Long Stride 
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    | Stride (32) | Cont. block size (12) | Reserved (4) | # blocks (12) | Reserved (4) |
    |                   Source addr                |
    | Stride (32) | Cont. block size (12) | Reserved (4) | # blocks (12) | Reserved (4) |
    |                   Dest.  addr                |
    |                   Handler args               |

    Long Vector 
    |0                             64|
    | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | Type (8) | # Args (8) |
    | # src vectors (4) | # dst vectors (4) | src Size 1 (12) | dst Size 1 (12) | Reserved (32) |
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

#endif