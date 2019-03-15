// the_gasnet_core_globals.c
// Toronto Heterogeneous GASNet
// Core API implementation
// (c)2011-2014 Ruediger Willenberg
#include <cstddef>
#include "thegasnet_globals.hpp"

// globally shared; built before first thread created
unsigned int gasnet_number_cpus = 2; // number of CPUs to use if affinity is used
unsigned int gasnet_ip_host_cnt = 0; // number of IP-connected hosts
gasnet_ip_host_t* gasnet_ip_hosts;
gasnet_node_routing_table_t *gasnet_node_routing_table;

// globally shared, only written in mutex-blocked threads 0 or [n]
gasnet_nodedata_t* gasnet_nodedata_all = NULL; // contains info about the current node
int gasnet_attach_count = 0;
bool gasnet_all_attached = false;
unsigned int gasnet_init_count = 0; // used to track threads as they initialize

// globally shared
mutex_t mutex_nodeInit; // mutex to initialize the node (per thread)
// thread_t* ipserver_listen_thread;

// written identically on each thread
gasnet_node_t gasnet_local_threads; // threads per node
gasnet_node_t gasnet_num_nodes; // total number of nodes on this system
bool gasnet_pin_memory = false; // whether to pin memory to RAM

// global; written identically on each thread
gasnet_seginfo_t* segment_table_lib; // array of shared address pointers

// thread_local; copied between related app and handler threads
__thread gasnet_nodedata_t* nodedata = NULL; // used to hold a thread-local copy of gasnet_nodedata_all

// globally shared; allocated once, written multiple times with identical data
void **handlertable = NULL; // provided by user application, handler functions to run

queue<char*>* ip_bufferfifo;
