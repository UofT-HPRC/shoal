#ifndef SHOAL_INCLUDE_THEGASNET_CORE_H_
#define SHOAL_INCLUDE_THEGASNET_CORE_H_

#include "platforms.hpp"
// #include "thegasnet_globals.hpp"
#include "config.hpp"

#define PACKET_BUFFERS_PER_THREAD 128
#define MAX_HANDLER_FUNCTIONS 256
#define DEFINED_HANDLER_FUNCTIONS 3

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

#if defined(__MICROBLAZE__)
	#define START_NODE_GLOBALS typedef volatile struct gasnet_nodeglobals_str

	#define END_NODE_GLOBALS gasnet_nodeglobals_str_t; \
		void *gasnet_ngaddr = NULL;
							   
	#define NODE_GLOBALS_INIT(STRNAME, inits...) gasnet_nodeglobals_str_t gasnet_nodeglobals_str_i = { inits }; \
		register gasnet_nodeglobals_str_t* STRNAME = &gasnet_nodeglobals_str_i; \
		gasnet_ngaddr = (void*)STRNAME; 
#endif

#ifdef __x86_64__ARM__ // malloc check!; 
	#define START_NODE_GLOBALS typedef struct gasnet_nodeglobals_str

	#define END_NODE_GLOBALS gasnet_nodeglobals_str_t; \
		__thread void *gasnet_ngaddr = NULL;
							   
	#define NODE_GLOBALS_INIT(STRNAME, inits...) register gasnet_nodeglobals_str_t* STRNAME = (gasnet_nodeglobals_str_t*)malloc(sizeof(gasnet_nodeglobals_str_t)); \
		if (STRNAME) *STRNAME = (gasnet_nodeglobals_str_t){ inits }; \
		gasnet_ngaddr = (void*)STRNAME;
#endif

#define USE_NODE_GLOBALS(STRNAME) gasnet_nodeglobals_str_t* STRNAME = (gasnet_nodeglobals_str_t*)gasnet_ngaddr;

#define gasnet_AMRequestShort0(dest, token, handler) \
	SendAM(AM_SHORT, dest, token, handler, NULL, 0, NULL, 0)
#define gasnet_AMRequestLong0(dest, token, handler, source_addr, nbytes, dest_addr) \
	SendAM(AM_LONG, dest, token, handler, source_addr, nbytes, dest_addr, 0)

void gasnet_getSegmentInfo (gasnet_seginfo_t *seginfo_table, int numentries);
int thegasnet_sharedMemoryReady(unsigned int ctrlnode);
int gasnet_init(int *argc, char ***argv);
gasnet_node_t gasnet_mynode();
gasnet_node_t gasnet_nodes();
void InternalBarrierUpdate();
void MemReadyBarrierUpdate();
void counterUpdate(int arg);
int gasnet_attach(gasnet_handlerentry_t *table, int numentries, uintptr_t segsize, uintptr_t minheapoffset);
int SendAM(unsigned int function, unsigned int destnode, unsigned int token, gasnet_handler_t handler, char *source_addr, size_t nbytes, char* dest_addr, unsigned int M, ...);
void internal_barrier();
void gasnet_exit(int exitcode);
void read_from_ip(int fd, char* buf, unsigned int numbytes);
void write_to_ip(int fd, char* buf, unsigned int numbytes);
void report_peer_connected(const struct sockaddr_in* sa, socklen_t salen);
void ipserver_listen();
char* read_queue(std::queue<char *>* q, mutex_t* lock);
void write_queue(std::queue<char *>* q, mutex_t* lock, char* data, int size);
void write_queue(std::queue<char *>* q, mutex_t* lock, char* data);
void ipserver(gasnet_ip_host_t& data);
void handler_thread(gasnet_nodedata_t& data);
int SendAM_vargs(unsigned int function, unsigned int destnode, int token, gasnet_handler_t handler, char *source_addr, size_t nbytes, char* dest_addr,
	size_t src_stride, size_t src_blk_size, size_t src_blk_num,
	size_t dst_stride, size_t dst_blk_size, size_t dst_blk_num,
	unsigned int M, va_list Vargs);

#endif // SHOAL_INCLUDE_THEGASNET_CORE_H_
