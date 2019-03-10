#if !defined(THE_GASNET_GLOBALS_H_)
#define THE_GASNET_GLOBALS_H_

#include "platforms.hpp"
#include "utilities.hpp"

typedef unsigned int gasnet_node_t;
typedef unsigned int gasnet_handler_t;

// defines information about the node
typedef struct gasnet_nodedata_str {
    int id;
    gasnet_node_t mynode;
    gasnet_node_t nodes;		
    void* node_ngaddr;
    mutex_t* condmutex;
    condition_t* condition;
    thread_t* handler_thread;
    // queue_t* tokenfifo;
    // short int *tokentable;
    std::queue<char *>* paramfifo;
    unsigned int gasnode_seginfo_cnt;
    unsigned int gasnode_seginfo_rcvd;

    int barrier_cnt;
    unsigned int mem_ready_barrier_cnt;
    unsigned int counter; // generic counter for H_ADD handler
    // TODO Add H_WAIT

    std::queue<char *>* bufferfifo;
    mutex_t* bufferfifo_out_lock;
    mutex_t* bufferfifo_in_lock;
    mutex_t* paramfifo_in_lock;
    mutex_t* paramfifo_out_lock;
} gasnet_nodedata_t;

typedef struct gasnet_ip_host_str {
    unsigned int ip4addr;

    int server_fd;
    int server_connected;
    thread_t* serverthread;

    struct sockaddr_in client_dest;
    int client_fd;
    int client_connected;
    mutex_t* IPsend_mutex;
} gasnet_ip_host_t;

typedef enum {none=0,thread,ip,fpga} gasnet_routing_type;
typedef struct gasnet_node_routing_table_str {
    gasnet_routing_type type;
    unsigned short int	index;
} gasnet_node_routing_table_t;

// defines shared memory size and address
typedef struct{
	void * addr; // void *addr; - differs from standard to accomodate different pointer sizes
	void * size; // uintptr_t size; - differs from standard to accomodate different pointer sizes
} gasnet_seginfo_t;

// globally shared; built before first thread created
extern unsigned int gasnet_number_cpus; // number of CPUs to use if affinity is used
extern unsigned int gasnet_ip_host_cnt; // number of IP-connected hosts
extern gasnet_ip_host_t* gasnet_ip_hosts;
extern gasnet_node_routing_table_t *gasnet_node_routing_table;

// globally shared, only written in mutex-blocked threads 0 or [n]
extern gasnet_nodedata_t* gasnet_nodedata_all; // contains info about the current node
extern int gasnet_attach_count; // counts the number of threads that have called gasnet_attach
extern bool gasnet_all_attached; // marked true when all threads have attached
extern unsigned int gasnet_init_count; // used to track threads as they initialize

// globally shared
extern mutex_t mutex_nodeInit; // mutex to initialize the node (per thread)
extern thread_t* ipserver_listen_thread;

// written identically on each thread
extern gasnet_node_t gasnet_local_threads; // threads per node
extern gasnet_node_t gasnet_num_nodes; // total number of nodes on this system
extern bool gasnet_pin_memory; // whether to pin memory to RAM

// global; written identically on each thread
extern gasnet_seginfo_t* segment_table_lib; // array of shared address pointers

// thread_local; copied between related app and handler threads
extern __thread gasnet_nodedata_t* nodedata; // thread-copy of gasnet_nodedata_all
extern __thread void* gasnet_ngaddr; // node_global_address - pointer to thread's shared variables

// globally shared; allocated once, written multiple times with identical data
extern void **handlertable; // provided by user application, handler functions to run

    #include <sys/socket.h> // defines sockets (must precede if*.h)
    #include <linux/if.h> // defines IFNAMSIZ
    #include <linux/if_ether.h> // defines ethhdr, ETH_DATA_LEN and ETH_FRAME_LEN
    #include <linux/if_packet.h> // defines saddrll
    #include <arpa/inet.h> // defines htons
    #include <stdio.h> // defines printf
    #include <string.h> // defines memset, strncpy
    #include <sys/ioctl.h> // defines ioctl, SIOCGIFINDEX, SIOCGIFHWADDR
    #include <unistd.h> // defines close
    #include <pthread.h> // needed for threads
    #include <thread> // defines std::thread
    #include <mutex> // defines mutexes
    #include <netinet/ether.h> // defines ether_header
    #include <list>
    #include <sys/epoll.h> 
    #include <fcntl.h>
    #include <netdb.h> // defines NI_MAXHOST, NI_MAXSERV, and getnameinfo()
    #include <assert.h>

    union eth_frame{
        struct{
            struct ethhdr    header;
            unsigned char    data[ETH_DATA_LEN];
        } field;
        unsigned char    buffer[ETH_FRAME_LEN];
    };

    #define AM_WORD_SIZE 8
    union AM_header{
        struct{
            char type;
            short src;
            short dst;
            short payload_size : 12;
            short handler : 4;
            char handler_args;
        } field;
        unsigned char buffer[AM_WORD_SIZE];
    };

    union AM_packet_ID{
        struct{
            char reserved [5];
            int packet_ID : 24;
        } field;
        struct{
            short stride;
            short block_size : 12;
            short block_num : 12;
            int packet_ID : 24;
        } field_str;
        struct{
            short reserved0 : 4;
            short dst_vectors : 4;
            short reserved1 : 12;
            short vec_size_1 : 12;
            char reserved2 : 8;
            int packet_ID : 24;
        } field_vec;
        unsigned char buffer[AM_WORD_SIZE];
    };

    #define ETH_PAYLOAD_SIZE (ETH_DATA_LEN-(2*AM_WORD_SIZE))
    union eth_packet{
        struct  {
            AM_header am_header;
            AM_packet_ID packet_id;
            char payload[ETH_PAYLOAD_SIZE];
        } field;
        char buffer[ETH_FRAME_LEN];
    };

    // 2^12 (width of payload) * 8 bytes/word
    #define AMMaxLongRequest 32768

    // 15 vectors (size + dst) = 29 words + 2 words (header + token) + 15 (handler args)
    #define MAX_HEADER_BYTES 368

    #define MAX_PAYLOAD_SIZE (AMMaxLongRequest+MAX_HEADER_BYTES)
    union AM_packet{
        struct  {
            AM_header am_header;
            AM_packet_ID packet_id;
            char payload[MAX_PAYLOAD_SIZE-(2*AM_WORD_SIZE)];
        } field;
        char buffer[MAX_PAYLOAD_SIZE];
    };

    extern std::queue<AM_packet> ip_bufferfifo;

    #define MAX_TCP_FDS 256 // ? why?

#endif // THE_GASNET_GLOBALS_H_
