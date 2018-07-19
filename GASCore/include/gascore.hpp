#ifndef GASCORE_H_
#define GASCORE_H_

//constant declarations
#define GC_BIG_ENDIAN 1
#define GC_M_AXI_ADDR_WIDTH 32
#define GC_M_AXI_DATA_WIDTH 32
#define GC_MAX_BURST_LEN 16
#define GC_MAX_PACKET_BYTES 8192

//derived constant declarations
#define GC_DATA_BYTES_PER_WORD (GC_M_AXI_DATA_WIDTH/4)

#endif