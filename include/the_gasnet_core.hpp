#if !defined(THE_GASNET_CORE_H_)
#define THE_GASNET_CORE_H_

#define PACKET_BUFFERS_PER_THREAD 128
#define MAX_HANDLER_FUNCTIONS 256

#define GASNET_OK 0

#define PACKET_BUFFERS_FOR_IP 32

#define GASNET_IP_SERVER_PORT 9886

#define MAX_QUEUE_SIZE 16


#ifdef __x86_64__ARM__
#include <sys/mman.h> // needed for mlock
#endif

typedef struct{
	gasnet_handler_t index; // == 0 for don’t care
	void (*fnptr)();
} gasnet_handlerentry_t;

#endif // THE_GASNET_CORE_H_
