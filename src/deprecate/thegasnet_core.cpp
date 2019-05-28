/////////////////////////////////////
// the_gasnet_core_multithreaded.c
// Toronto Heterogeneous GASNet
// Core API implementation
// (c)2011-2014 Ruediger Willenberg

//modified by Varun
/////////////////////////////////////

// #define _GNU_SOURCE
// #include <sched.h>

#include <cstddef>
#include "platforms.hpp"
#include "thegasnet_globals.hpp"
#include "thegasnet_core.hpp"
#include "active_messages.hpp"
#include "user_config.hpp"


#include <stdio.h>

#ifdef __ARM__
	// #include "xillybus_lite_fsl.h"
#endif

// #include "the_gasnet_core.h"
// #include "the_gasnet_core_globals.h"
// #include "thread_fifo.h"

//? removed TCP heartbeat method

#ifdef SET_CPU_AFFINITY
unsigned int print_cpu_affinity(char* name, unsigned int id){
	cpu_set_t *cpusetp;
	size_t size;
	unsigned int largest_cpu_number = 0;
	
	cpusetp = CPU_ALLOC(gasnet_number_cpus);
	size = CPU_ALLOC_SIZE(gasnet_number_cpus);
	pthread_getaffinity_np(pthread_self(), size, cpusetp);
	
	char setoutput[40];
	int cindex;
	for (cindex=0;cindex<gasnet_number_cpus;cindex++)
		if (CPU_ISSET_S(cindex,size,cpusetp))
		{
			setoutput[cindex]='1'; largest_cpu_number = cindex;
		}
		else
			setoutput[cindex]='.';
	setoutput[cindex]='\0';
	printf("%s %d: %s\r\n",name,id,setoutput);
	return largest_cpu_number;
}


static void limit_cpu_affinity(unsigned int cpus, unsigned int thread, unsigned int handler)
{
	cpu_set_t *cpusetp;
	size_t size;
	
	cpusetp = CPU_ALLOC(gasnet_number_cpus);
	size = CPU_ALLOC_SIZE(gasnet_number_cpus);
	CPU_ZERO_S(size, cpusetp);
	
	
	
	if (gasnet_local_threads<cpus) // separate handler cpus - good idea? Thinking about hyperthreading.
	{
		thread = thread + gasnet_local_threads;
	}
	CPU_SET_S(thread % cpus,size, cpusetp);
	pthread_setaffinity_np(pthread_self(), size, cpusetp);
}
#endif


//? deleted make_packet_buffer() -- unneeded with std::queue

void gasnet_getSegmentInfo (gasnet_seginfo_t *seginfo_table, int numentries)
{
	
	// Long term: Tree based seginfo distribution (like efficient barriers - READ STUFF)
	unsigned int t;
	if (gasnet_mynode()==0)
	{
		/// DEACTIVATED SEGMENT SHARING
		/*
		// wait for incoming ReqS (should count up to nodecounts)
		while(READ_SHARED(nodedata->gasnode_seginfo_cnt) < (gasnet_nodes()-1));

		// sent out tabledata to everybody else(ReqM)
		for (t=1;t<gasnet_nodes();t++)
			gasnet_AMRequestMedium0(t,hc_DistributeSegmentTable,segment_table_lib,gasnet_nodes()*sizeof(gasnet_seginfo_t));
		*/
		///
	}
	else
	{
		/// DEACTIVATED SEGMENT SHARING
		/*
		gasnet_handlerarg_t segaddrL = ptr64_gargL(segment_table_lib[gasnet_mynode()].addr);
		gasnet_handlerarg_t segaddrH = ptr64_gargH(segment_table_lib[gasnet_mynode()].addr);
		gasnet_handlerarg_t segsizeL = ptr64_gargL(segment_table_lib[gasnet_mynode()].size);
		gasnet_handlerarg_t segsizeH = ptr64_gargH(segment_table_lib[gasnet_mynode()].size);
		// send local data to 0
		gasnet_AMRequestShort5(0,hc_SendSegmentInfo,gasnet_mynode(),segaddrH,segaddrL,segsizeH,segsizeL);
		// receive table
		while (!READ_SHARED(nodedata->gasnode_seginfo_rcvd));
		*/
		///
	}
	
	/// WORKAROUND FOR NO SEGMENT SHARING
	// if (numentries<gasnet_nodes()) // silly GASNet spec idiosyncrasy that numentries could be smaller
	// {
	// 	///memcpy(seginfo_table, (void*)&segment_table_lib[0], numentries*sizeof(gasnet_seginfo_t));
	// 	for (t=0;t<numentries;t++)
	// 		if (t==gasnet_mynode())
	// 		{
	// 			seginfo_table[t] = segment_table_lib[t];	
	// 		}
	// 		else
	// 		{
	// 			seginfo_table[t].addr = (voidp64)0;
	// 			seginfo_table[t].size = segment_table_lib[gasnet_mynode()].size; //  ! implying all segments are the same size !
	// 		}
	// }
	// else
	// {
		///memcpy(seginfo_table, (void*)&segment_table_lib[0], gasnet_nodes()*sizeof(gasnet_seginfo_t));
		for (t=0;t<gasnet_nodes();t++)
			if (t==gasnet_mynode())
			{
				seginfo_table[t] = segment_table_lib[t];	
			}
			else
			{
				seginfo_table[t].addr = (void *)0;
				seginfo_table[t].size = segment_table_lib[gasnet_mynode()].size; //  ! implying all segments are the same size !
			}
	// }

	/// internal_barrier();
	
	// return GASNET_OK;
}

int thegasnet_sharedMemoryReady(unsigned int ctrlnode)
{
	int t;
	
	if (gasnet_mynode()==ctrlnode)
	{
		printf("Enter thegasnet_sharedMemoryReady\n");
		while(READ_SHARED(nodedata->mem_ready_barrier_cnt)!=(gasnet_nodes())-1) sched_yield();
	COMPILER_BARRIER
		nodedata->mem_ready_barrier_cnt=0;
	COMPILER_BARRIER
		for(t=0;t<gasnet_nodes();t++)
			if (t != ctrlnode)
				gasnet_AMRequestShort0(t, t, H_INCR_MEM);
	}
	else
	{
		// Send message to ctrl node
		gasnet_AMRequestShort0(ctrlnode, ctrlnode, H_INCR_MEM);
		while(READ_SHARED(nodedata->mem_ready_barrier_cnt)==0) sched_yield();
	COMPILER_BARRIER
		nodedata->mem_ready_barrier_cnt=0;
	COMPILER_BARRIER
	}
	
	return GASNET_OK;
}


int gasnet_init(int *argc, char ***argv){
	lock_t lock(mutex_nodeInit, std::defer_lock);
	lock.lock();

	int parsed_node = -1;

	// parse/remove arguments
	for (int t=1;t<(*argc);t++){
		int remove_arg = 0;
		
		// pore over arguments; use the ones relevant here; for those, set remove_arg=1
		if (strncmp("--GASNET_NUM_NODES=",(*argv)[t],19)==0){
			remove_arg = 1;
			gasnet_num_nodes = atoi(&(*argv)[t][19]);
		}
		else if (strncmp("--GASNET_LOCAL_THREADS=",(*argv)[t],23)==0){
			remove_arg = 1;
			gasnet_local_threads = atoi(&(*argv)[t][23]);
		}
		else if (strncmp("--GASNET_NODE=",(*argv)[t],14)==0){
			remove_arg = 1;
			parsed_node = atoi(&(*argv)[t][14]);
		}
		else if (strcmp("--GASNET_PIN_MEM",(*argv)[t])==0){
			remove_arg = 1;
			gasnet_pin_memory= 1;
		}
		
		if (remove_arg){
			for(int u=t ; u<((*argc)-1) ; u++) // copy remaining arguments one down
				(*argv)[u] = (*argv)[u+1];
			t--; // repeat this loop iteration - new argument in this position
			(*argc)--; // reduce number of arguments
		}
	}

	// first thread init
	if (gasnet_init_count == 0){
		gasnet_nodedata_all = (gasnet_nodedata_t*) malloc(gasnet_local_threads * sizeof(gasnet_nodedata_t));
		for (int t=0; t<gasnet_local_threads; t++){
			gasnet_nodedata_all[t].id = t;
			gasnet_nodedata_all[t].gasnode_seginfo_cnt = 0;
			gasnet_nodedata_all[t].gasnode_seginfo_rcvd = 0;
			gasnet_nodedata_all[t].condmutex = new mutex_t;
			// gasnet_nodedata_all[t].bufferfifo_out_lock = new mutex_t;
			// gasnet_nodedata_all[t].bufferfifo_in_lock = new mutex_t;
			// gasnet_nodedata_all[t].paramfifo_in_lock = new mutex_t;
			// gasnet_nodedata_all[t].paramfifo_out_lock = new mutex_t;
			gasnet_nodedata_all[t].paramfifo = new queue<char*>(MAX_QUEUE_SIZE, true);
			gasnet_nodedata_all[t].bufferfifo = new queue<char*>(MAX_QUEUE_SIZE, true);
			gasnet_nodedata_all[t].condition = new condition_t;
			char* bufferfifo_alloc = (char*) malloc(MAX_QUEUE_SIZE*sizeof(AM_packet));
			for(int i = 0; i < MAX_QUEUE_SIZE; i++){
				(gasnet_nodedata_all[t].bufferfifo)->write(bufferfifo_alloc);
				bufferfifo_alloc += sizeof(AM_packet);
			}
		}
			
		// other steps that should only be done once
		segment_table_lib = (gasnet_seginfo_t*) calloc(gasnet_num_nodes,sizeof(gasnet_seginfo_t));
	}
		
		nodedata = &gasnet_nodedata_all[gasnet_init_count];
		nodedata->node_ngaddr = gasnet_ngaddr;
		nodedata->mynode = parsed_node;
		nodedata->barrier_cnt = 0;
		nodedata->mem_ready_barrier_cnt = 0;

		// printf("Creating handler thread %d\n", nodedata->mynode);
		nodedata->handler_thread = new std::thread(handler_thread, std::ref(*nodedata));
		(nodedata->handler_thread)->detach();
		// printf("Detached handler thread %d\n", nodedata->mynode);
		COMPILER_BARRIER
		gasnet_init_count++;
		COMPILER_BARRIER

		lock.unlock();

	return GASNET_OK;
}

void allocate_handlerTable(){
	lock_guard_t guard(mutex_nodeInit);
	if (!handlertable){
		handlertable = (void **)calloc(MAX_HANDLER_FUNCTIONS,sizeof(void*));
		if (!handlertable){
			fprintf(stderr,"Handlertable could not be allocated.\r\n");
			exit(1);
		}
	}
}

gasnet_node_t gasnet_mynode(){
	return nodedata->mynode;
}

gasnet_node_t gasnet_nodes(){
	return gasnet_num_nodes;
}

void InternalBarrierUpdate(){
	nodedata->barrier_cnt++;
	return;
}

void MemReadyBarrierUpdate(){
	nodedata->mem_ready_barrier_cnt++;
	return;
}

void counterUpdate(int arg){
	nodedata->counter+=arg;
	return;
}

int gasnet_attach(gasnet_handlerentry_t *table, int numentries, uintptr_t segsize, uintptr_t minheapoffset){
	unsigned int t;

	// build handler table
	while(READ_SHARED(gasnet_init_count)<gasnet_local_threads);
	allocate_handlerTable();
	
	// fill in application handlers
	for(t=0;t<numentries;t++)
		handlertable[table[t].index] = (void*) table[t].fnptr;

	// fill in private GASNet handlers
	handlertable[H_INCR_BAR] = (void*)InternalBarrierUpdate;
	handlertable[H_INCR_MEM] = (void*)MemReadyBarrierUpdate;
	handlertable[H_ADD] = (void*)counterUpdate;
	
	// reserve shared memory - clearing is _not_ part of the GASNet standard
	void *mem_segment;

	lock_t lock(mutex_nodeInit, std::defer_lock);
	lock.lock();
	
	mem_segment = malloc((size_t)segsize);
	if (!mem_segment){
		fprintf(stderr,"Shared segment could not be allocated.\r\n");
		abort();
	}
	else if (gasnet_pin_memory){
		if (mlock(mem_segment,(size_t)segsize) != 0){
			//perror(NULL);
			fprintf(stderr,"Pinning shared memory in RAM did not succeed.\r\n");
			abort();
		}
	}
	
	segment_table_lib[gasnet_mynode()].addr = (void *) mem_segment,
	segment_table_lib[gasnet_mynode()].size = (void *) segsize;


	gasnet_attach_count++;
	// last thread attach - activate message input threads
	if (gasnet_attach_count==gasnet_local_threads){
		
		COMPILER_BARRIER
			gasnet_all_attached = true;
		COMPILER_BARRIER

		// if (gasnet_local_fpga)
		// 	pthread_create (&check_fpga_thread, NULL,(void *)&check_fpga, NULL);
			
		// create server listen thread -> creates server threads when accepting connections
		// ip_bufferfifo = make_packet_buffer(AMMaxLongRequest+MAX_HEADER_BYTES, PACKET_BUFFERS_FOR_IP, "IP_bufferfifo"); // 8kB packets, largest LongV header, 8 buffers
		ip_bufferfifo = new queue<char*>(MAX_QUEUE_SIZE, true);
		char* bufferfifo_alloc = (char*) malloc(MAX_QUEUE_SIZE*sizeof(AM_packet));
		for(int i = 0; i < MAX_QUEUE_SIZE; i++){
			ip_bufferfifo->write(bufferfifo_alloc);
			bufferfifo_alloc += sizeof(AM_packet);
		}

		// pthread_create (&ipserver_listen_thread, NULL,(void *)&ipserver_listen, NULL);
		// printf("Creating ipserver listen thread\n");
		thread_t ipserver_listen_thread = std::thread(ipserver_listen);
		
		// loop: connect to servers, sleep shortly when failing
		int connect_count = 0;
		int connect_loops_count = 0;
		
		for(t=0;t<gasnet_ip_host_cnt;t++) // address structure prep
		{
			if((gasnet_ip_hosts[t].client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				fprintf(stderr,"Error : Could not create client socket.\r\n");
				exit(1);
			} 
			gasnet_ip_hosts[t].client_dest.sin_family = AF_INET; 
			gasnet_ip_hosts[t].client_dest.sin_port   = htons(GASNET_IP_SERVER_PORT); 
			gasnet_ip_hosts[t].client_dest.sin_addr.s_addr = gasnet_ip_hosts[t].ip4addr;
		}
			
		while(connect_count < gasnet_ip_host_cnt) // until all connected
		{
			for(t=0;t<gasnet_ip_host_cnt;t++)
				if (!gasnet_ip_hosts[t].client_connected)// only try unconnected ones
				{
					if( connect(gasnet_ip_hosts[t].client_fd,
						(struct sockaddr *)&gasnet_ip_hosts[t].client_dest,
						sizeof(struct sockaddr_in))  >=  0)
					{
#ifdef NO_TCP_DELAY
						int tcpnodelaypar = 1;
						if (( setsockopt(gasnet_ip_hosts[t].client_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&tcpnodelaypar, sizeof(tcpnodelaypar))) == -1)
						{
							fprintf(stderr,"Setting client socket TCP_NODELAY failed.\r\n");
							exit(1);
						}
#endif
						gasnet_ip_hosts[t].client_connected = 1;
						connect_count++;
						//fprintf(stderr,"Connect to host %d succeeded.\r\n",t);
					}
					else
					{
						//perror(NULL);
						//fprintf(stderr,"Connect to host %d failed.\r\n",t);
					}
				}

			connect_loops_count++;
			
			if (connect_loops_count==16) // too many IP connection attempts
			{
				fprintf(stderr,"Error: Too many IP connection attempts failed.\r\n");
				exit(1);
			}
			
			if (connect_count < gasnet_ip_host_cnt)
				sleep(1); // wait a second!
		}
		
		// join ipserver listen thread after it finished
		// pthread_join (ipserver_listen_thread, NULL);
		ipserver_listen_thread.join();
		// printf("Joined ipserver listen thread\n");
	
		// start TCP heartbeat
		// pthread_create (&tcp_heartbeat_thread, NULL,(void *)&tcp_heartbeat, NULL);
	}
		
	// pthread_mutex_unlock(&nodeinit_mutex);
	lock.unlock();
	
#ifdef SET_CPU_AFFINITY
	limit_cpu_affinity(gasnet_number_cpus, nodedata->mynode, 0);
	print_cpu_affinity("CPU set of application node", nodedata->mynode);
#endif // SET_CPU_AFFINITY

	return GASNET_OK;
}

int SendAM(unsigned int function, unsigned int destnode, unsigned int token, gasnet_handler_t handler, char *source_addr, size_t nbytes, char* dest_addr, unsigned int M, ...)
{
	va_list Vargs;
    va_start(Vargs, M);
   	int retval = SendAM_vargs(function, destnode, token, handler, source_addr, nbytes, dest_addr, 0, 0, 0, 0, 0, 0, M, Vargs);
    va_end(Vargs);
	return retval;
}


// Other internal functions
void internal_barrier()
{
	unsigned int t;

	unsigned int nodes  = gasnet_num_nodes;
	unsigned int mynode = gasnet_mynode();

	if (mynode==0)
	{
		while(READ_SHARED(nodedata->barrier_cnt)!=(nodes)-1) sched_yield();
		nodedata->barrier_cnt=-1;
		for(t=1;t<nodes;t++)
			gasnet_AMRequestShort0(t,-1, H_INCR_BAR);
	}
	else
	{
		// Send message to 0
		gasnet_AMRequestShort0(0,-1,H_INCR_BAR);
		while(READ_SHARED(nodedata->barrier_cnt)==0) sched_yield();
		nodedata->barrier_cnt=-1;
	}

	return;
}

void gasnet_exit(int exitcode) // behaviour not complying with GASNet standard (is collective right now)
{
	internal_barrier();
	
	// Sequence:
	// * Cancel request to handler thread
	// * handler thread join
	// * increase exit count
	// * if last thread:
	// ** request IP and FPGA thread cancels

	//~ This doesn't work because gasnet_mynode() needs to be offset by the 
	//~ lowest index on this node to get the correct array index. This information
	//~ is available only in main_wrapper.
	// gasnet_nodedata_t* nodedata_tmp;
	// nodedata_tmp = &gasnet_nodedata_all[gasnet_mynode()];
	// (nodedata_tmp->condition)->notify_all();

	//~ This doesn't work because server thread is detached
	// if (gasnet_mynode() == 0){
	// 	for(int i = 0; i < gasnet_ip_host_cnt; i++){
	// 		(gasnet_ip_hosts[i]).serverthread->join();
	// 	}
	// }

	// TODO: delete objects created with new
}


void read_from_ip(int fd, char* buf, unsigned int numbytes)
{
	int bread;
	
	while(numbytes)
	{
		bread = read(fd, buf, numbytes);
		
		if ((bread<0) && (errno == EINTR))
			continue;
		
		if (bread<0)
		{
			fprintf(stderr,"TCP connection timed out.");//"IP read failure.\r\n");
			//perror(NULL);
			exit(1);
		}

		// printf("Read: ");
		// for(int i = 0; i < bread; i++){
		// 	printf("%x", buf[i]);
		// }
		// printf(" (%d bytes)\n", bread);
		
		numbytes -= bread;
		buf += bread;
	}
}


void write_to_ip(int fd, char* buf, unsigned int numbytes)
{
	int written;
	
	while(1) // this while(1)/break approach allows writing a 0 to flush
	{
		written = write(fd, buf, numbytes);
		
		if ((written<0) && (errno == EINTR))
			continue;
		
		if (written<0)
		{
			fprintf(stderr,"PCIe write failure.\r\n");
			exit(1); // best solution (open ports?)
		}

		// printf("Wrote: ");
		// for(int i = 0; i < written; i++){
		// 	printf("%x", buf[i]);
		// }
		// printf(" (%d bytes)\n", written);
		
		numbytes -= written; buf += written;
		if (numbytes==0) break;
	}
}

void report_peer_connected(const struct sockaddr_in* sa, socklen_t salen){
	char hostbuf[NI_MAXHOST];
	char portbuf[NI_MAXSERV];
	if (getnameinfo((struct sockaddr*)sa, salen, hostbuf, NI_MAXHOST, portbuf, NI_MAXSERV, 0) == 0) {
		printf("peer (%s, %s) connected\n", hostbuf, portbuf);
	} else {
		printf("peer (unknonwn) connected\n");
	}
}

// accepting ip connections
void ipserver_listen()
{
	printf("Running ipserver_listen thread\n");
	struct timeval timeout;      
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	
	int listenfd;
	int acceptfd;
	struct sockaddr_in listen_addr; 
	struct sockaddr_in accept_addr; 
	
	int t;
	
    if (( listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr,"Listen socket creation failed.\r\n");
		exit(1);
	}
	
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_port = htons(GASNET_IP_SERVER_PORT); 
	
	int so_reuseaddr = 1;
	
    if (( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&so_reuseaddr, sizeof(so_reuseaddr))) == -1)
	{
		fprintf(stderr,"Setting listen socket options failed.\r\n");
		exit(1);
	}

    if (( setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout))) == -1)
	{
		fprintf(stderr,"Setting listen socket options failed.\r\n");
		exit(1);
	}
	#ifdef NO_TCP_DELAY
		int tcpnodelaypar = 1;
		if (( setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, (void*)&tcpnodelaypar, sizeof(tcpnodelaypar))) == -1)
		{
			fprintf(stderr,"Setting listen socket TCP_NODELAY failed.\r\n");
			exit(1);
		}
	#endif	
    if (( bind(listenfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) == -1)
	{
		fprintf(stderr,"Binding address to socket failed.\r\n");
		exit(1);
	}
	
    listen(listenfd, 32); 

	int connect_count = 0;
	
	while(connect_count < gasnet_ip_host_cnt) // until all connected
	{
		socklen_t aasize = sizeof(accept_addr);
		
		if (( acceptfd = accept(listenfd, (struct sockaddr*)&accept_addr, &aasize) ) >= 0) //worked
		{
			for(t=0;t<gasnet_ip_host_cnt;t++)
			{
				if (accept_addr.sin_addr.s_addr == gasnet_ip_hosts[t].ip4addr)
				{
					if (gasnet_ip_hosts[t].server_connected)
					{
						fprintf(stderr,"Error: IP client connecting twice.\r\n");
						exit(1);
					}
					else
					{
						gasnet_ip_hosts[t].server_connected = 1;
						gasnet_ip_hosts[t].server_fd = acceptfd;
						connect_count++;
						break;
					}
				}
			}
			
			if (t==gasnet_ip_host_cnt)
			{
				fprintf(stderr,"Error: Client IP 0x%X unknown.\r\n",accept_addr.sin_addr.s_addr);
				exit(1);
			}
			#ifdef NO_TCP_DELAY
				tcpnodelaypar = 1;
				if (( setsockopt(acceptfd, IPPROTO_TCP, TCP_NODELAY, (void*)&tcpnodelaypar, sizeof(tcpnodelaypar))) == -1)
				{
					fprintf(stderr,"Setting accept socket TCP_NODELAY failed.\r\n");
					exit(1);
				}
			#endif
			// pthread_create (&gasnet_ip_hosts[t].serverthread, NULL, (void*)ipserver, (void*)&gasnet_ip_hosts[t]);
			// printf("Creating ipserver thread");
			gasnet_ip_hosts[t].serverthread = new std::thread(ipserver, std::ref(gasnet_ip_hosts[t]));
			(gasnet_ip_hosts[t]).serverthread->detach();
			// printf("Detached ipserver thread");
		}
		else 
		{
			fprintf(stderr,"Server accept timeout.\r\n");
			exit(1);
		}
	}
}

// char * read_queue(std::queue<char *>* q, mutex_t* lock){
// 	lock_guard_t guard(*lock);
// 	while (q->empty()){}

// 	char* element = reinterpret_cast<char *>(&(q->front()));
// 	q->pop();
// 	return element;
// }

// void write_queue(std::queue<char *>* q, mutex_t* lock, char* data, int size){
// 	lock_guard_t guard(*lock);
// 	while (q->size() > MAX_QUEUE_SIZE){}
// 	AM_packet temp;
// 	for(int i = 0; i < size; i++)
// 		temp.buffer[i] = *(data++);
// 	q->push((char *) &temp);	
// }

// void write_queue(std::queue<char *>* q, mutex_t* lock, char* data){
// 	lock_guard_t guard(*lock);
// 	while (q->size() > MAX_QUEUE_SIZE){}
// 	q->push(data);	
// }

// ip server thread
void ipserver(gasnet_ip_host_t& data)
{
	
	long long header0;
	unsigned int msgtype;
	unsigned int destnode;
	unsigned int wordsize;
	char* packet_buffer;
	unsigned int dest_thread;
	gasnet_nodedata_t *dest_nodedata;
	
	gasnet_ip_host_t *connection_data = &data;
	int read_fd = connection_data->server_fd;
	
	
	while(1)
	{
		// read header - should suspend thread on empty channel
		read_from_ip(read_fd,reinterpret_cast<char*>(&header0),AM_WORD_SIZE);
		
		if (header0==0xFFFFFFFF) // TCP heartbeat -> no processing, suspend again
			continue;

		
		// destnode = (header0 & 0x00FF0000) >> 16;
		// wordsize =  header0 & 0x0000FFFF;
		// printf("header read: %lld\n", header0);
		// printf("header read le: %lld\n", htole64(header0));
		// printf("header read be: %lld\n", htobe64(header0));
		destnode = (header0 & AM_DST_BITMASK) >> AM_DST_LOWER;
		msgtype = (header0 & AM_TYPE_BITMASK) >> AM_TYPE_LOWER;
		wordsize = (header0 & AM_PAYLOAD_SIZE_BITMASK) >> AM_PAYLOAD_SIZE_LOWER;

		// printf("destnode: %d\n", destnode);
		// printf("msgtype: %d\n", msgtype);
		// printf("wordsize: %d\n", wordsize);

		//// BUFFER ALLOCATION FOR PACKET
		if ( (gasnet_node_routing_table[destnode].type == ip) || // covers arriving data from local loopback
		     (gasnet_node_routing_table[destnode].type == thread) ) // IP-to-thread  (no IP-IP, would cause eternal loopback)
		{
			// determine destnode struct
			for(dest_thread=0 ; dest_thread<gasnet_local_threads ; dest_thread++)
				if (gasnet_nodedata_all[dest_thread].mynode == destnode)
					break;
			if (dest_thread==gasnet_local_threads)
				fprintf(stderr,"can't find destination thread for destnode %d, header0: %X\r\n",destnode,header0);

			dest_nodedata = &gasnet_nodedata_all[dest_thread];
			
			// packet_buffer = ptfifo_read(dest_nodedata->bufferfifo,1); // blocking
			// printf("thread %d, fifo %d", gasnet_mynode(), dest_nodedata->bufferfifo->size());
			// packet_buffer = read_queue(dest_nodedata->bufferfifo, (dest_nodedata->bufferfifo_out_lock));
			packet_buffer = (dest_nodedata->bufferfifo)->read();
			// printf("thread %d, fifo %d", gasnet_mynode(), dest_nodedata->bufferfifo->size());
		}
		// else if (gasnet_node_routing_table[destnode].type == fpga) // IP-to-FPGA (x86-64: PCIe)
		// {
		// 	packet_buffer = ptfifo_read(fpga_bufferfifo,1); // blocking
		// }

		// reading IP packet data to buffer
		*(long long*)packet_buffer = header0;
		read_from_ip(read_fd,packet_buffer+AM_WORD_SIZE, wordsize-AM_WORD_SIZE);

		// DISTRIBUTION TO DIFFERENT SOURCES
		if ( (gasnet_node_routing_table[destnode].type == ip) || // covers arriving data from local loopback
		     (gasnet_node_routing_table[destnode].type == thread) ) // IP-to-thread  (no IP-IP, would cause eternal loopback)
		{
			// handler data transfer
			// pthread_mutex_lock(&dest_nodedata->condmutex);
			// ptfifo_write(dest_nodedata->paramfifo,packet_buffer,1);

			// pthread_cond_signal(&dest_nodedata->condition);
			// pthread_mutex_unlock(&dest_nodedata->condmutex);
			lock_t lock(*(dest_nodedata->condmutex), std::defer_lock);
			lock.lock();
			// write_queue(dest_nodedata->paramfifo,(dest_nodedata->paramfifo_in_lock), packet_buffer);
			dest_nodedata->paramfifo->write(packet_buffer);
			(dest_nodedata->condition)->notify_one();
			lock.unlock();
		}
		// else if (gasnet_node_routing_table[destnode].type == fpga) // IP-to-FPGA (x86-64: PCIe)
		// {
		// 	pthread_mutex_lock(&fpga_write_mutex);

		// 	// SENDING HEADERS
		// 	#ifdef __ARM__
		// 		cputfsl(header0,1);
		// 		write_to_fpga(packet_buffer+4, wordsize*4); // same for all
		// 	#endif // ARM
			
		// 	#ifdef __x86_64__
		// 		write_to_fpga(packet_buffer, (wordsize+1)*4); // same for all

		// 		// FLUSH PCIe
		// 		//write_to_fpga(NULL, 0); // fails 
		// 		close(pcie_write);
		// 		pcie_write = open("/dev/xillybus_write_32", O_WRONLY);
		// 		if (pcie_write < 0)
		// 		{
		// 			fprintf(stderr,"PCIe write device not open.\r\n");
		// 			exit(1); // best solution? (open ports?)
		// 		}
		// 	#endif

		// 	pthread_mutex_unlock(&fpga_write_mutex);

		// 	ptfifo_write(fpga_bufferfifo,packet_buffer,1);
		// }
	}
}

void handler_thread(gasnet_nodedata_t& data)
{
	struct timespec waittime = { .tv_sec = 1, .tv_nsec = /*1*/00000000 };
	
	unsigned int t;

	void *packet_buffer;
	long long *packet_buffer_uint;
	unsigned int headersize;
	unsigned int argoffset;
	unsigned int wordsize;
	long payload_offset;
	void *payload;
	
	unsigned int srcnode;
	gasnet_token_t token;

	unsigned int function;
	unsigned int numargs;
	gasnet_handler_t handler;

	size_t bsize = 0;

	gasnet_handlerarg_t *arg;

	unsigned int destaddrL;
	unsigned int destaddrH;
	unsigned int dest_sizeX;
	unsigned int dest_sizeY;
	unsigned int dest_strideY;

	void *bufaddr = NULL;

	// nodedata = (gasnet_nodedata_t*) data;  // making data structure shared with app_thread thread-local
	nodedata = &data;
	gasnet_ngaddr = nodedata->node_ngaddr; // copying app thread data pointer to handler thread data pointer

	while(!READ_SHARED(gasnet_all_attached)); // only start receiving when all local threads are attached

#ifdef SET_CPU_AFFINITY
	limit_cpu_affinity(gasnet_number_cpus, nodedata->mynode, 1);
	print_cpu_affinity("CPU set of handler node", nodedata->mynode);
#endif // SET_CPU_AFFINITY

	// pthread_mutex_lock(&nodedata->condmutex);
	lock_t lock(*(nodedata->condmutex), std::defer_lock);
	lock.lock();

	while(nodedata->barrier_cnt!=-1){
		while ((nodedata->paramfifo)->empty() && nodedata->barrier_cnt!=-1){

			// int waitreturn = pthread_cond_wait(&nodedata->condition, &nodedata->condmutex);//);
			(nodedata->condition)->wait(lock);
		}
		if(nodedata->barrier_cnt==-1) break;
		//while (ptfifo_empty(nodedata->paramfifo));

		printf("handling recv data\n");
		// get packet buffer address
		// packet_buffer = ptfifo_read(nodedata->paramfifo,0);
		// packet_buffer = (nodedata->paramfifo)->front();
		// (nodedata->paramfifo)->pop();
		packet_buffer = (nodedata->paramfifo)->read();
		packet_buffer_uint = (long long*)packet_buffer;

		// extract message parameters
		// srcnode  = hdextract(packet_buffer_uint,nnSrcnode); //destnode = hdextract(packet_buffer_uint,nnDestnode);
		srcnode = (packet_buffer_uint[0] & AM_SRC_BITMASK) >> AM_SRC_LOWER;
		function = (packet_buffer_uint[0] & AM_TYPE_BITMASK) >> AM_TYPE_LOWER;
		numargs = (packet_buffer_uint[0] & AM_HANDLER_ARGS_BITMASK) >> AM_HANDLER_ARGS_LOWER;
		handler = (packet_buffer_uint[0] & AM_HANDLER_BITMASK) >> AM_HANDLER_LOWER;

		// printf("header read in handler: %lld\n", packet_buffer_uint[0]);
		// printf("header read in handler le: %lld\n", htole64(packet_buffer_uint[0]));
		// printf("header read in handler be: %lld\n", htobe64(packet_buffer_uint[0]));

		int token = (packet_buffer_uint[1] & AM_TOKEN_BITMASK) >> AM_TOKEN_LOWER;
		// printf("srcnode: %d, function: %d, numargs: %d, handler: %d\n", srcnode,function,numargs,handler);

		// function = hdextract(packet_buffer_uint,nnFunction);
		// numargs  = hdextract(packet_buffer_uint,nnNumargs);
		// handler  = hdextract(packet_buffer_uint,nnHandler);

		// HEADER AND PACKET SIZE DETERMINATION - without first word (NetIf header: src,dest,#words)
		if (isShortAM(function))  		headersize = 2;
		if (isMediumAM(function)) 		headersize = 2;
		if (isLongAM(function))  		headersize = 3;
		if (isLongStridedAM(function))	headersize = 3;

		//if (isLongVectoredAM(function))   headersize = ?;
		argoffset = headersize + 1;
		arg = (gasnet_handlerarg_t*)&(packet_buffer_uint[argoffset]);
		payload_offset = (long)((headersize+numargs)*AM_WORD_SIZE);
		payload = packet_buffer + payload_offset;

		// printf("srcnode: %d\n", srcnode);
		// printf("function: %d\n", function);
		// printf("numargs: %d\n", numargs);
		// printf("handler: %d\n", handler);
		
		if (!isShortAM(function)){
			// bsize = hdextract(packet_buffer_uint,nnBsize);
			bsize = ((packet_buffer_uint[0] & AM_PAYLOAD_SIZE_BITMASK) >> AM_PAYLOAD_SIZE_LOWER) - \
				numargs - headersize;
		}
		
		if (isMediumAM(function)){
			bufaddr = payload; // temporary buffer is in memblock
		}
		
		if (isLongAM(function)){
			// destaddrL = hdextract(packet_buffer_uint,nnDestaddrL);
			// destaddrH = hdextract(packet_buffer_uint,nnDestaddrH);

			// bufaddr  = (void*)uintHL_ptr(destaddrH,destaddrL)
			// 			+ (long)segment_table_lib[gasnet_mynode()].addr;
			bufaddr = (long long *)(packet_buffer_uint[2] + (long long)segment_table_lib[gasnet_mynode()].addr);
			if (gasnet_node_routing_table[srcnode].type != thread) // already copied if local
				memcpy(bufaddr, payload, bsize);
		}
			
		// if (isLongStridedAM(function))
		// {
		// 	dest_sizeX   = packet_buffer_uint[3];
		// 	dest_sizeY   = packet_buffer_uint[4];
		// 	destaddrL    = packet_buffer_uint[5];
		// 	destaddrH    = packet_buffer_uint[6];
		// 	dest_strideY = packet_buffer_uint[7];

		// 	bufaddr  = (void*)uintHL_ptr(destaddrH,destaddrL)
		// 				+ (long)segment_table_lib[gasnet_mynode()].addr;
		// 	void *dest_addr = bufaddr;
		// 	unsigned int LS;
			
		// 	if (!(gasnet_node_routing_table[srcnode].type == thread)) // already copied if local
		// 	{
		// 		for(LS=0;LS<dest_sizeY;LS++)
		// 		{
		// 			memcpy(dest_addr,payload,dest_sizeX);
		// 			payload     += (long)dest_sizeX;
		// 			dest_addr += (long)dest_strideY;
		// 		}
		// 	}
		// }

		// Token
		// token    = Source_To_Token((short int)srcnode);		
		
		// handler calls
		if (isShortAM(function))
			switch(numargs)
			{
				case 0:
					((void(*)(gasnet_token_t
								))handlertable[handler])(token);break;
				case 1:
					((void(*)(gasnet_token_t,garg
								))handlertable[handler])(token,arg[0]);break;
				case 2:
					((void(*)(gasnet_token_t,garg,garg
								))handlertable[handler])(token,arg[0],arg[1]);break;
				case 3:
					((void(*)(gasnet_token_t,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2]);break;
				case 4:
					((void(*)(gasnet_token_t,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3]);break;
				case 5:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4]);break;
				case 6:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5]);break;
				case 7:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6]);break;
				case 8:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7]);break;
				case 9:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8]);break;
				case 10:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9]);break;
				case 11:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10]);break;
				case 12:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11]);break;
				case 13:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12]);break;
				case 14:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13]);break;
				case 15:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13],arg[14]);break;
				case 16:
					((void(*)(gasnet_token_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13],arg[14],arg[15]);break;
				default:
					break;
			}
		else
			switch(numargs)
			{
				case 0:
					((void(*)(gasnet_token_t,void*,size_t
								))handlertable[handler])(token,bufaddr,bsize);break;
				case 1:
					((void(*)(gasnet_token_t,void*,size_t,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0]);break;
				case 2:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1]);break;
				case 3:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2]);break;
				case 4:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3]);break;
				case 5:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4]);break;
				case 6:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5]);break;
				case 7:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6]);break;
				case 8:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7]);break;
				case 9:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8]);break;
				case 10:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9]);break;
				case 11:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10]);break;
				case 12:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11]);break;
				case 13:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12]);break;
				case 14:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13]);break;
				case 15:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13],arg[14]);break;
				case 16:
					((void(*)(gasnet_token_t,void*,size_t,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg,garg
								))handlertable[handler])(token,bufaddr,bsize,arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7],arg[8],arg[9],arg[10],arg[11],arg[12],arg[13],arg[14],arg[15]);break;
				default:
					break;
			}
		
		//re-circulate block address
		// ptfifo_write(nodedata->bufferfifo,packet_buffer,1);
		// write_queue(nodedata->bufferfifo, (nodedata->bufferfifo_in_lock), (char*)packet_buffer);
		nodedata->bufferfifo->write((char*) packet_buffer);
		//re-circulate token
		// tfifo_write(nodedata->tokenfifo, token,1);

		
	}

	printf("exiting handler thread %d\n", gasnet_mynode());
	// pthread_mutex_unlock(&nodedata->condmutex);
	lock.unlock();
}


// gasnet_token_t Source_To_Token(short int source)
// {
// 	gasnet_token_t token = tfifo_read(nodedata->tokenfifo,1);
// 	nodedata->tokentable[token] = source;
// 	return token;
// }

// short int Token_To_Destination(gasnet_token_t token)
// {
// 	short int destination = nodedata->tokentable[token];
// 	return destination;
// }


// void check_fpga(void *data)
// {
// 	unsigned int header0;
// 	unsigned int destnode;
// 	unsigned int wordsize;
// 	void *packet_buffer;
// 	unsigned int dest_thread;
// 	gasnet_nodedata_t *dest_nodedata;
	
// #ifdef __x86_64__
// 	if (pcie_read < 0)
// 	{
// 		fprintf(stderr,"PCIe read device not open.\r\n");
// 		exit(1); // best solution (open ports?)
// 	}
// #endif
// #ifdef __ARM__
// 	//TODO: ARM Status Check
// #endif // ARM
// 	while(1)
// 	{
// #ifdef __x86_64__
// 		read_from_fpga((void*)&header0, 1*4); // get first word of GAScore network packet
// #endif
// #ifdef __ARM__
// 		// checking for available header
// 		xlfsl_irqwait();
// 		// get header from FSL1
// 		getfsl(header0,1);
// #endif // ARM


// 		destnode = (header0 & 0x00FF0000) >> 16;
// 		wordsize =  header0 & 0x0000FFFF;
// 		//// BUFFER ALLOCATION FOR PACKET
// 		if (gasnet_node_routing_table[destnode].type == thread) // FPGA-to-thread
// 		{
// 			// determine destnode struct
// 			for(dest_thread=0 ; dest_thread<gasnet_local_threads ; dest_thread++)
// 				if (gasnet_nodedata_all[dest_thread].mynode == destnode)
// 					break;
				
// 			dest_nodedata = &gasnet_nodedata_all[dest_thread];
			
// 			packet_buffer = ptfifo_read(dest_nodedata->bufferfifo,1); // blocking
// 		}
// 		else if (gasnet_node_routing_table[destnode].type == ip) // FPGA-to-IP
// 		{
// 			packet_buffer = ptfifo_read(ip_bufferfifo,1); // blocking
// 		}
// 		else
// 		{
// 			fprintf(stderr,"checkFPGA: illegal destination\r\n");
// 		}
		
		
// 		// reading FPGA packet data to buffer
// 		*(unsigned int*)packet_buffer = header0;
// 		read_from_fpga(packet_buffer+4, wordsize*4);

		
// 		if (gasnet_node_routing_table[destnode].type == thread) // thread-to-thread
// 		{
// 			// handler data transfer
// 			pthread_mutex_lock(&dest_nodedata->condmutex);
// 			ptfifo_write(dest_nodedata->paramfifo,packet_buffer,1);

// 			pthread_cond_signal(&dest_nodedata->condition);
// 			pthread_mutex_unlock(&dest_nodedata->condmutex);
// 		}
// 		else if (gasnet_node_routing_table[destnode].type == ip) // FPGA-to-IP
// 		{	
// 			gasnet_ip_host_t* dest_host = &gasnet_ip_hosts[gasnet_node_routing_table[destnode].index];
			
// 			// spinlock on IP connection establishment - TODO Better solution?
// 			while(!READ_SHARED(dest_host->client_connected));
// 			int write_fd = dest_host->client_fd;

// 			// SEND PACKET
// 			pthread_mutex_lock(&dest_host->IPsend_mutex);
// 				write_to_ip(write_fd, packet_buffer, (wordsize+1)*4);
// 			pthread_mutex_unlock(&dest_host->IPsend_mutex);
			
// 			//re-circulate block address
// 			ptfifo_write(ip_bufferfifo,packet_buffer,1);
// 		}
// 	}
// }


#ifdef __x86_64__
	// void read_from_fpga(void* buf, unsigned int numbytes)
	// {
	// 	int bread;
	// 	int fill = numbytes % 4;
		
	// 	while(numbytes)
	// 	{
	// 		bread = read(pcie_read, buf, numbytes);
			
	// 		if ((bread<0) && (errno == EINTR))
	// 			continue;
			
	// 		if (bread<0)
	// 		{
	// 			fprintf(stderr,"PCIe read failure.\r\n");
	// 			exit(1); // best solution (open ports?)
	// 		}
			
	// 		numbytes -= bread; buf += bread;
	// 	}
		
	// 	if (!fill) return;

	// 	// if not modulo-4-bytes, read leftover bytes
	// 	unsigned int filldata;
	// 	void* fillptr = (void*)&filldata;
	// 	fill = 4-fill;

	// 	while(fill)
	// 	{
	// 		bread = read(pcie_read, fillptr, fill);
			
	// 		if ((bread<0) && (errno == EINTR))
	// 			continue;
			
	// 		if (bread<0)
	// 		{
	// 			fprintf(stderr,"PCIe read failure.\r\n");
	// 			exit(1); // best solution (open ports?)
	// 		}
			
	// 		fill -= bread; fillptr += bread;
	// 	}
		
	// }

	// void write_to_fpga(void* buf, unsigned int numbytes)
	// {
	// 	int written;
	// 	int fill = numbytes % 4;
		
	// 	while(1) // this while(1)/break approach allows writing a 0 to flush
	// 	{
	// 		written = write(pcie_write, buf, numbytes);
			
	// 		if ((written<0) && (errno == EINTR))
	// 			continue;
			
	// 		if (written<0)
	// 		{
	// 			fprintf(stderr,"PCIe write failure.\r\n");
	// 			exit(1);
	// 		}
			
	// 		numbytes -= written; buf += written;
	// 		if (numbytes==0) break;
	// 	}

	// 	if (!fill) return;

	// 	// if not a modulo-4-bytes, write leftover bytes

	// 	unsigned int filldata = 0;
	// 	void* fillptr = (void*)&filldata;
	// 	fill = 4-fill;
		
	// 	while(fill)
	// 	{
	// 		written = write(pcie_write, fillptr, fill);
			
	// 		if ((written<0) && (errno == EINTR))
	// 			continue;
			
	// 		if (written<0)
	// 		{
	// 			fprintf(stderr,"PCIe write failure.\r\n");
	// 			exit(1);
	// 		}
			
	// 		fill -= written; fillptr += written;
	// 	}
	// }
#endif


#ifdef __ARM__
	void read_from_fpga(void* buf, unsigned int numbytes)
	{

		if (((unsigned long)buf) % 4)
		{
			fprintf(stderr,"Xillybus Lite FSL: Buffer not word-aligned; not supported yet.\r\n");
			exit(1); // best solution (open ports?)
		}

		while(numbytes >= 4)
		{
			getfsl(*buf,1); // blocking; spin-locking in xillybus_lite_fsl.c
			
			numbytes -= 4; buf += 4;
		}
		if (numbytes > 0)
		{
			unsigned int t, rdata;
			getfsl(rdata,1);
			
			for(t=0; t<numbytes; t++)
			{
				((unsigned char*)buf)[t] = ((unsigned char*)&rdata)[t];
			}
		}
        
	}

	void write_to_fpga(void* buf, unsigned int numbytes)
	{
		if (((unsigned long)buf) % 4)
		{
			fprintf(stderr,"Xillybus FSL: Buffer not word-aligned; not supported yet.\r\n");
			exit(1); // best solution (open ports?)
		}

		while(numbytes >= 4)
		{
			putfsl(*(unsigned int*)buf,1); // blocking; spin-locking in xillybus_lite_fsl.c
			
			numbytes -= 4; buf += 4;
		}
		if (numbytes > 0)
		{
			unsigned int t, wdata = 0;
			
			for(t=0; t<numbytes; t++)
			{
				((unsigned char*)&wdata)[t] = ((unsigned char*)buf)[t];
				//wdata = ((*(unsigned int*)buf) & 0xF) & wdata;

			}

			putfsl(wdata,1);
		}
	}
#endif // ARM


// int SendAM_vargs(unsigned int function, unsigned int dest_or_token, gasnet_handler_t handler, void *source_addr, size_t nbytes, voidp64 dest_addr,
// 					size_t source_strideY, size_t source_sizeX, size_t source_sizeY,
// 					size_t dest_strideY, size_t dest_sizeX, size_t dest_sizeY,
// 					unsigned int M, va_list Vargs)
// {
int SendAM_vargs(unsigned int function, unsigned int destnode, int token, gasnet_handler_t handler, char *source_addr, size_t nbytes, char* dest_addr,
					size_t src_stride, size_t src_blk_size, size_t src_blk_num,
					size_t dst_stride, size_t dst_blk_size, size_t dst_blk_num,
					unsigned int M, va_list Vargs)
{
	unsigned int t;
	void *packet_buffer;
	long long *packet_buffer_uint;
	unsigned int headersize;
	unsigned int argoffset;
	unsigned int wordsize;
	long long payload_offset;
	char *payload;
	// unsigned int destnode;
	unsigned int dest_thread;
	gasnet_nodedata_t *dest_nodedata;
	
	// HEADER AND PACKET SIZE DETERMINATION - without first word (NetIf header: src,dest,#words)
	if (isShortAM(function))  		headersize = 2;
	else if (isMediumAM(function)) 		headersize = 2;
	else if (isLongAM(function))  		headersize = 3;
	else if (isLongStridedAM(function))	headersize = 3;
	else
		assert(0);
	//if (isLongVectoredAM(function))   headersize = ?;
	argoffset = headersize + 1;
	wordsize = headersize + M; // args
	
	payload_offset = (long long)((wordsize)*AM_WORD_SIZE);
	// wordsize += (nbytes+3) / 4; // data   TODO: use word-fillup on IP? relevant for PCIe re-routing
	wordsize = (wordsize*AM_WORD_SIZE) + nbytes;

	// Destination retrieval if reply
	// if (isReplyAM(function))
	// 	destnode = Token_To_Destination(dest_or_token);
	// else						
	// 	destnode = dest_or_token;
	
	//// BUFFER ALLOCATION FOR PACKET
	if (gasnet_node_routing_table[destnode].type == ip) // thread-to-IP
	{
		gasnet_ip_host_t* dest_host = &gasnet_ip_hosts[gasnet_node_routing_table[destnode].index];
		
		// spinlock on IP connection establishment - TODO Better solution?
		while(!READ_SHARED(dest_host->client_connected));
		int write_fd = dest_host->client_fd;

		packet_buffer = ip_bufferfifo->read(); // blocking
		// packet_buffer = new AM_packet();
		packet_buffer_uint = (long long*)packet_buffer;
	
		payload = (char*)packet_buffer + payload_offset;
	
	}
	else if (gasnet_node_routing_table[destnode].type == thread) // thread-to-thread
	{
		// determine destnode struct
		for(dest_thread=0 ; dest_thread<gasnet_local_threads ; dest_thread++)
			if (gasnet_nodedata_all[dest_thread].mynode == destnode)
				break;
			
		dest_nodedata = &gasnet_nodedata_all[dest_thread];
		
		// packet_buffer = read_queue(dest_nodedata->bufferfifo,dest_nodedata->bufferfifo_out_lock); // blocking
		packet_buffer = dest_nodedata->bufferfifo->read();
		// packet_buffer = new AM_packet;
		packet_buffer_uint = (long long*)packet_buffer;
		payload = (char*)packet_buffer + payload_offset;
	}
	// else if (gasnet_node_routing_table[destnode].type == fpga) // thread-to-FPGA (x86-64: PCIe)
	// {
	// 	packet_buffer = ptfifo_read(fpga_bufferfifo,1); // blocking
	// 	packet_buffer_uint = (unsigned int*)packet_buffer;
	// 	payload = packet_buffer + payload_offset;
	// }

	//////////////// PACKET CREATION ////////////////////
	for(t=0;t<=headersize;t++)
		packet_buffer_uint[t] = 0;
		
	// word 0
	// hdencode(packet_buffer_uint,nnSrcnode,gasnet_mynode());
	// hdencode(packet_buffer_uint,nnDestnode,destnode);
	// hdencode(packet_buffer_uint,nnWords,wordsize);
	packet_buffer_uint[0] = createHeader(gasnet_mynode(),destnode,wordsize,handler,function,M);
	// printf("header write: %lld\n", packet_buffer_uint[0]);
	// printf("header write le: %lld\n", htole64(packet_buffer_uint[0]));
	// printf("header write be: %lld\n", htobe64(packet_buffer_uint[0]));
	
	// word 1
	// hdencode(packet_buffer_uint,nnFunction,function);
	// hdencode(packet_buffer_uint,nnNumargs,function);
	// hdencode(packet_buffer_uint,nnHandler,handler);
	
	// word 2
	// if (!isShortAM(function))
	// {
	// 	hdencode(packet_buffer_uint,nnBsize,nbytes);
	// }

	if (!isLongVectoredAM(function) && !isLongStridedAM(function))
	{
		packet_buffer_uint[1] = createToken(token);
	}
	
	if (isLongAM(function) || isLongStridedAM(function))
	{
		// hdencode(packet_buffer_uint,nnDestaddrL, ptr64_uintL(dest_addr) );
		// hdencode(packet_buffer_uint,nnDestaddrH, ptr64_uintH(dest_addr) );
		packet_buffer_uint[2] = (long long) dest_addr;

	}
	
	if (isLongStridedAM(function))
	{
		packet_buffer_uint[1] = createStrided(dst_stride, dst_blk_size, dst_blk_num, token);
		// packet_buffer_uint[3] = (unsigned int)dest_sizeX;
		// packet_buffer_uint[4] = (unsigned int)dest_sizeY;
		// packet_buffer_uint[5] = ptr64_uintL(dest_addr);
		// packet_buffer_uint[6] = ptr64_uintH(dest_addr);
		// packet_buffer_uint[7] = (unsigned int)dest_strideY;
	}
	
	for(t=0; t<M; t++)
	{
		packet_buffer_uint[argoffset+t] = va_arg (Vargs, long long);
	}

	if (isMediumAM(function))
	{
		memcpy(payload,source_addr,nbytes); // TODO: Direct copy for small amounts?
	}		

	dest_addr += (long)segment_table_lib[destnode].addr; // only relevant for direct local copy

	if (isLongAM(function))
	{
		if (gasnet_node_routing_table[destnode].type == thread) // local copy directly
			memcpy(dest_addr,source_addr,nbytes);
		else
			memcpy(payload,source_addr,nbytes);
	}

	if (isLongStridedAM(function))
	{
		unsigned int LS;
		
		if (gasnet_node_routing_table[destnode].type != thread)
		{
			for(LS=0;LS<src_blk_num;LS++)
			{
				memcpy(payload,source_addr,src_blk_size);
				payload     += (long)src_blk_size;
				source_addr += (long)src_stride;
			}
		}
		else  // local copy directly
		{
			if ((src_blk_num==dst_blk_num) && (src_blk_size==dst_blk_size))
			{
				for(LS=0;LS<src_blk_num;LS++)
				{
					memcpy(dest_addr,source_addr,dst_blk_size);
					source_addr += (long)src_blk_size;
					dest_addr += (long)dst_stride;
				}
			}
			else // replace with more sophisticated direct copy algorithm
					// -- this one is way more expensive than unpack on destnode!
			{
				// for(LS=0;LS<source_sizeY;LS++)
				// {
				// 	memcpy(payload,source_addr,source_sizeX);
				// 	payload     += (long)source_sizeX;
				// 	source_addr += (long)source_strideY;
				// }
				// for(LS=0;LS<dest_sizeY;LS++)
				// {
				// 	memcpy(dest_addr,payload,dest_sizeX);
				// 	payload     += (long)dest_sizeX;
				// 	dest_addr += (long)dest_strideY;
				// }
				assert(false);
			}
		}
	}
	
	if (gasnet_node_routing_table[destnode].type == ip) // thread-to-IP
	{	
		gasnet_ip_host_t* dest_host = &gasnet_ip_hosts[gasnet_node_routing_table[destnode].index];
		
		// spinlock on IP connection establishment - TODO Better solution?
		while(!READ_SHARED(dest_host->client_connected));
		int write_fd = dest_host->client_fd;

		// SEND PACKET
		// pthread_mutex_lock(&dest_host->IPsend_mutex);
		lock_t lock(*(dest_host->IPsend_mutex), std::defer_lock);
		lock.lock();
			// printf("Writing %d bytes\n", wordsize);
			// printf("Function: %d\n", function);
			write_to_ip(write_fd, (char*)packet_buffer, wordsize);
		lock.unlock();
		// pthread_mutex_unlock(&dest_host->IPsend_mutex);

		//re-circulate block address
		// ptfifo_write(ip_bufferfifo,packet_buffer,1);
	}
	else if (gasnet_node_routing_table[destnode].type == thread) // thread-to-thread
	{
		// handler data transfer
		lock_t lock(*(dest_nodedata->condmutex), std::defer_lock);
		lock.lock();
		// pthread_mutex_lock(&dest_nodedata->condmutex);
		// ptfifo_write(dest_nodedata->paramfifo,packet_buffer,1);
		// write_queue(dest_nodedata->paramfifo, dest_nodedata->paramfifo_in_lock, (char*)packet_buffer);
		dest_nodedata->paramfifo->write((char*) packet_buffer);

		// pthread_cond_signal(&dest_nodedata->condition);
		(dest_nodedata->condition)->notify_one();
		// pthread_mutex_unlock(&dest_nodedata->condmutex);
		lock.unlock();
	}
	// else if (gasnet_node_routing_table[destnode].type == fpga) // thread-to-FPGA (x86-64: PCIe)
	// {
	// 	pthread_mutex_lock(&fpga_write_mutex);

	// 	// SENDING HEADERS
	// 	#ifdef __ARM__
	// 		cputfsl(packet_buffer_uint[0],1);
	// 		write_to_fpga((void*)&(packet_buffer_uint[1]), wordsize*4); // same for all
	// 	#endif // ARM
		
	// 	#ifdef __x86_64__
	// 		write_to_fpga(packet_buffer, (wordsize+1)*4); // same for all

	// 		// FLUSH PCIe
	// 		//write_to_fpga(NULL, 0); // fails 
	// 		close(pcie_write);
	// 		pcie_write = open("/dev/xillybus_write_32", O_WRONLY);
	// 		if (pcie_write < 0)
	// 		{
	// 			fprintf(stderr,"PCIe write device not open.\r\n");
	// 			exit(1); // best solution? (open ports?)
	// 		}
	// 	#endif

	// 	pthread_mutex_unlock(&fpga_write_mutex);
		
	// 	//re-circulate block address
	// 	ptfifo_write(fpga_bufferfifo,packet_buffer,1);
	// }


	return GASNET_OK;
}
