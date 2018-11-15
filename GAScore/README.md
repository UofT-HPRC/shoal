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
    | Type (8) | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | # Args (8) |
    | Reserved (40) | Packet ID (24) |
    |                   Handler args               |
    
    Medium
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | # Args (8) |
    | Reserved (40) | Packet ID (24) |
    |                   Handler args               |
    |                   Payload ...                |

    Long
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | # Args (8) |
    | Reserved (40) | Packet ID (24) |
    |                   Destination                |
    |                   Handler args               |
    |                   Payload ...                |

    Long Stride 
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | # Args (8) |
    | Stride (16) | Cont. block size (12) | blocks (12) | Packet ID (24) |
    |                   Destination                |
    |                   Handler args               |
    |                   Payload ...                |

    Long Vector 
    |0                             64|
    | Type (8) | SRC (16) | DST (16) | Payload (words) (12) | Handler (4) | # Args (8) |
    | Reserved (4) | # dst vectors (4) | Reserved (12) | Size 1 (12) | Reserved (8) | Packet ID (24) |
    |                   Destination                |
    | Size 2... (12) | Reserved (52) |
    |                   Destination                |
    ...
    |                   Handler args               |
    |                   Payload ...                |
</pre>