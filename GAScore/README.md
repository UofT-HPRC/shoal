# GAScore

Type:  
 * Bit 0: set if short or long strided  
 * Bit 1: set if medium or long vectored
 * Bit 2: set if a longx message
 * Bit 3: reserved
 * Bit 4: set if data payload for medium/long message comes from a FIFO
 * Bit 5: set if a medium/long message is Async
 * Bit 6: set if message is a reply
 * Bit 7: reserved

<pre>
Active Message Packet Schema

    Short
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Reserved (48) | Packet ID (16) |
    |                   Handler args               |

    Medium
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Reserved (48) | Packet ID (16) |
    |                   Handler args               |
    |                   Payload ...                |

    Long
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Reserved (48) | Packet ID (16) |
    |                   Destination                |
    |                   Handler args               |
    |                   Payload ...                |

    Long Stride (stride in bytes, block size in bytes)
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Stride (12) | Cont. block size (16) | blocks (12) | Reserved (8) | Packet ID (16) |
    |                   Destination                |
    |                   Handler args               |
    |                   Payload ...                |

    Long Vector 
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Reserved (4) | # dst vectors (4) | Reserved (12) | Size 1 (12) | Reserved (16) | Packet ID (16) |
    |                   Destination                |
    | Size 2... (12) | Reserved (52) |
    |                   Destination                |
    ...
    |                   Handler args               |
    |                   Payload ...                |


Short
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Reserved (48) | Packet ID (16) |
    |                   Handler args               |
    
    Medium (FIFO 0)
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Reserved (48) | Packet ID (16) |
    |                   SRC addr                   |
    |                   Handler args               |
    Medium (FIFO 1)
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Reserved (48) | Packet ID (16) |
    |                   Handler args               |
    |                   Payload ...                |
    Long (FIFO 0)
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Reserved (48) | Packet ID (16) |
    |                   Source Addr                |
    |                   Destination                |
    |                   Handler args               |
    Long (FIFO 1)
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Reserved (48) | Packet ID (16) |
    |                   Destination                |
    |                   Handler args               |
    |                   Payload ...                |
    Long Stride 
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | Stride (12) | Cont. block size (16) | blocks (12) | Reserved (24) |
    |                   Source addr                |
    | Stride (12) | Cont. block size (16) | blocks (12) | Reserved (8) | Packet ID (16) |
    |                   Dest.  addr                |
    |                   Handler args               |
    Long Vector 
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Message Size (bytes) (16) | Handler (4) | # Args (4) |
    | # src vectors (4) | # dst vectors (4) | src Size 1 (12) | dst Size 1 (12) | Reserved (16) | Packet ID (16) |
    |                   src   addr.                |
    |                   dst   addr.                |
    | src size 2... (12) | Reserved (52) |
    |                   src   addr.                |
    ...
    | dst size 2... (12) | Reserved (52) |
    |                   dst   addr.                |
    ...    
    |                   Handler args               |

    Reply
    |0                             64|
    | Type (8) | SRC (16) | Message Size (bytes) (16) | Reserved (8) | Packet ID (16) |
</pre>
