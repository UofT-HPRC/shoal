#if !defined(SHOAL_REGISTER_MAP_H_)
#define SHOAL_REGISTER_MAP_H_

#define COUNTER_THRESHOLD_ADDR 0x10 //32 bit interrupt threshold

/*
Bit 0: enable (active high)
Bit 1: reset (active high)
Bit 2-31: reserved
*/
#define COUNTER_ENABLE_ADDR 0x18
#define COUNTER_ENABLE 0x1
#define COUNTER_DISABLE 0x0
#define COUNTER_RESET 0x2

#endif // SHOAL_REGISTER_MAP_H_
