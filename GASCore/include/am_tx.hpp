#ifndef AM_TX_H_
#define AM_TX_H_

#include "globals.hpp"
#include "packet_defs.hpp"

/* AM Request Packet Schema

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

#endif