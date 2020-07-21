#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
// #include "the_gasnet_core.h"
// #include "the_gasnet_extended.h"
// #include "platforms.h"
#include <math.h>

#include "jacobi.hpp"

#define PRINTCOND (myid==ctrlnode)
#define TIMECOND (myid==0)

#undef  INTERMEDIATE_TIMES

#define CELLTYPE int64_t
int wait = 0;
// HANDLER FUNCTION PROTOTYPES AND TABLE
#define hc_L0_update_aura 132
void L0_update_aura(gasnet_token_t token, void *buf, size_t nbytes);

#define hc_S0_barrier_increment 140
void S0_barrier_increment(gasnet_token_t token);

#define hc_M0_compnode_parameters 160
void M0_compnode_parameters(gasnet_token_t token, void *buf, size_t nbytes);

#define hc_S4_send_stats 161
void S4_send_stats(gasnet_token_t token, gasnet_handlerarg_t arg0, gasnet_handlerarg_t arg1, gasnet_handlerarg_t arg2, gasnet_handlerarg_t arg3);

#define hc_S1_stats_reply 162
void S1_stats_reply(gasnet_token_t token, gasnet_handlerarg_t arg0);

// static gasnet_handlerentry_t handlers[] =
// {

//     // THE_GASNET_EXTENDED_HANDLERS
// 	{
// 		hc_L0_update_aura,
// 		(void(*)())L0_update_aura
// 	},
// 	{
// 		hc_S0_barrier_increment,
// 		(void(*)())S0_barrier_increment
// 	},
// 	{
// 		hc_M0_compnode_parameters,
// 		(void(*)())M0_compnode_parameters
// 	},
// 	{
// 		hc_S4_send_stats,
// 		(void(*)())S4_send_stats
// 	},
// 	{
// 		hc_S1_stats_reply,
// 		(void(*)())S1_stats_reply
// 	}
// };

#ifdef __HLS__
// this function, if inlined, or placed in code, behaves badly in Verilog. In
// simulation, it does do three writes but they appear as burst writes to the wrong
// addresses. Doing it this way seems to work properly. Tested in HLS 2018.1
void start_timer(volatile int* axi_timer){
    axi_timer[0] = 0x0; // stop timer
    axi_timer[1] = 0; // set load register to 0
    axi_timer[0] = 0x20; // load timer with load register
    axi_timer[0] = 0x80; // start timer
}

word_t get_time(volatile int* axi_timer){
    // #pragma HLS INLINE
    // axi_timer[0] = 0x0; // stop timer
    word_t time = *(axi_timer + 0x2); // read timer count
	return time;
}

word_t accum_time(word_t old_time, volatile int* axi_timer){
	word_t time = get_time(axi_timer);
	return old_time + time;
}

void send_time(shoal::kernel* kernel, gc_AMToken_t token, word_t time, volatile int* axi_timer){
    word_t dumb = *(axi_timer);
    if(dumb != 0xFF){
    kernel->sendMediumAM_async(KERNEL_NUM_TOTAL-1, token, H_EMPTY, 0, NULL, 8);
    kernel->sendPayload(0, time, true); // assume always send to kernel 0
    }
    // axi_timer[0] = 0x0; // stop timer
    // kernel->wait_reply(1, axi_timer);
}
#else
auto get_time(){
    return std::chrono::high_resolution_clock::now();
}

auto get_time_diff(time_point_t timer){
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - timer;
    return elapsed.count();
}

double accum_time(time_point_t old_time, double sum_time){
	double delta = get_time_diff(old_time);
	double new_sum = sum_time + delta;
	return new_sum;
}
#endif


// void barrier(unsigned int ctrlnode);


unsigned int log2floor(unsigned int val)
{
	int log = 0;
	val >>= 1;
	while(val)
	{
		log++;
		val >>= 1;
	}
	return log;
}

unsigned int powerof2(unsigned int val)
{
	unsigned int i;
	unsigned int res = 1;
	
	for(i=0 ; i<val ; i++)
		res *= 2;
	
	return res;
}

unsigned int ispowerof2(unsigned int val)
{
	if ( powerof2(log2floor(val)) == val)
		return 1;
	else
		return 0;
}


typedef struct pole
{
	int32_t charge;
	unsigned int xpos;
	unsigned int ypos;
	unsigned int valid;
} pole_str;

// right now, this works only because this is 320 bytes or 40 64 bit words even
// it needs to be aligned to the word boundary
typedef struct node_jacobidata
{
	uint32_t xsize;
	uint32_t ysize;
	uint32_t pole1xy;
	int32_t pole1value;
	uint32_t pole2xy;
	int32_t pole2value;
	uint32_t pole3xy;
	int32_t pole3value;
	uint32_t pole4xy;
	int32_t pole4value;
} node_jacobidata_str;


// START_NODE_GLOBALS

// 	unsigned int received;
// 	char *rcvstring;

// 	unsigned int barrier_cnt;
// 	unsigned int mynode,nodes;
	
// 	unsigned int bytes_received;
// 	//unsigned int bytes_receivedB;

// 	unsigned int ready;
// 	//unsigned int readyB;
	
// 	unsigned int expected_neighbour_bytes;
	
// 	unsigned int compnodes;
// 	unsigned int compnode0;
// 	unsigned int ctrlnode;
// 	unsigned int my_compnode;
	
// 	node_jacobidata_str njdata_local;
// 	unsigned int njdata_local_received;
	
// 	unsigned int stats_cnt;
// 	uint64_t *stats_squarederror;
// 	uint32_t *stats_gradient;
// 	unsigned int continue_running;

// END_NODE_GLOBALS;


extern "C"{
void jacobi(
    short id,
    galapagos::interface <word_t> * in,
    #ifdef __HLS__
	galapagos::interface<word_t> * out,
    volatile int * handler_ctrl,
	volatile int * axi_timer,
    word_t * local_mem
	#else
	galapagos::interface<word_t> * out
    #endif
){
    #pragma HLS INTERFACE axis port=in
    #pragma HLS INTERFACE axis port=out
    #pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_stable port=id
    #pragma HLS INTERFACE m_axi port=handler_ctrl depth=4096 offset=0
    #pragma HLS INTERFACE m_axi port=axi_timer depth=4096 offset=0
    // #pragma HLS INTERFACE m_axi port=instr_mem depth=4096 offset=0
    #pragma HLS INTERFACE m_axi port=local_mem depth=4096 offset=0
	// GLOBALS SHARED WITH HANDLER FUNCTIONS
	// NODE_GLOBALS_INIT(shared, .barrier_cnt=0, .received=0, .bytes_received=0, .ready=0,  \
	// 							 .njdata_local_received=0, .stats_cnt=0, .continue_running=0)

	int t,u,x,y,v,w;

	#define MAXPOLES 4
	pole_str poles[MAXPOLES] =
	{
		{
			// possibly change from xy coordinate to node ID
			100000000,	// charge +200M
			2,		// X = 2000
			2,		// Y = 2000
			1			// valid
		},
		{
			-100000000,	// charge -200M
			5,		// X = 2100
			6,		// Y = 2100
			1			// valid
		},
		{
			50000000,	// charge -100M
			6,		// X = 1000
			6,		// Y = 3000
			1			// valid
		},
		{ 0,0,0,0 }
	};

	// Time measurement variables
	unsigned int commsecs = 0, commusecs = 0;
	unsigned int compsecs = 0, compusecs = 0;
	unsigned int dispsecs = 0, dispusecs = 0;
	unsigned int bar1secs = 0, bar1usecs = 0;
	unsigned int bar2secs = 0, bar2usecs = 0;

	#ifdef __HLS__
	word_t commtime = 0;
	word_t comptime = 0;
	word_t bar1time = 0;
	word_t bar2time = 0;
	#else
	double commtime = 0;
	double comptime = 0;
	double bar1time = 0;
	double bar2time = 0;
	#endif
	
	// LOCAL VARIABLES
	char* mysharedmem = NULL;

	// pre-GASnet settings
	PRINT("Here we go...\r\n");
	
	

	// Timer_Init();
	
	// GASNET INIT
	// gasnet_init(&argc, &argv);
	#ifdef __HLS__
    shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out, handler_ctrl);
    #else
    shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out);
    #endif
	kernel.init();
	// gasnet_extended_init();
	// myid = myid;
	// shared->nodes = gasnet_nodes();
	
	/////////////////////////////////////
	// DEFINITION OF NODE DISTRIBUTION //
	/////////////////////////////////////
	
	// 1 CONTROL NODE; (NODES()-1) COMPNODES ; 0 is first compnode
	// (KERNEL_NUM_TOTAL-1) = shared->nodes - 1; 	// e.g. 16 nodes
	int myid = kernel.get_id();
	int compnode0 = 0;					// e.g. compnodes 0..15
	int ctrlnode  = KERNEL_NUM_TOTAL - 1; 	// e.g. Node #16
	int my_compnode = myid - compnode0;
	
	// Get app arguments
	int a,X=GRID_SIZE,Y=GRID_SIZE;
	
	// for (a=0 ; a<argc ; a++)
	// {
	// 	// pore over arguments; use the ones relevant here; for those, set remove_arg=1
	// 	if (strncmp("--X=",argv[a],4)==0)
	// 		X = atoi(&argv[a][4]);
	// 	else if (strncmp("--Y=",argv[a],4)==0)
	// 		Y = atoi(&argv[a][4]);
		
	// 	PRINT("Arg %d: %s\r\n",a,argv[a]);
	// }
	
	if (X==0)
		{ PRINT("X=0 illegal.\r\n"); abort(); }
	if (Y==0)
		{ PRINT("Y=0 illegal.\r\n"); abort(); }

		// Determine data size / distribution
	if (!ispowerof2((KERNEL_NUM_TOTAL-1)))
		{ PRINT("Number of nodes not power of 2.\r\n"); abort(); }
	
	/// 2D-Topology: if x!=y then x=2*y  -  TODO other way makes more sense for memory transfer optimization
	unsigned int log2nodes = log2floor((KERNEL_NUM_TOTAL-1));
	unsigned int xnodes = (powerof2(log2nodes/2 + log2nodes%2));
	unsigned int ynodes = (powerof2(log2nodes/2));
	
	// Fields per node
	if (X % xnodes)
		{ PRINT("X(%d) not divisible by horizontal node dimension(%d) \r\n",X,xnodes); abort(); }
	if (Y % ynodes)
		{ PRINT("Y(%d) not divisible by vertical node dimension(%d) \r\n",Y,ynodes); abort(); }
		
		
	///////////////////////////////////
	// MEMORY ALLOCATION
	///////////////////////////////////
		
	// memory requirements
	//size_t mem_available = 0x2000000 / (KERNEL_NUM_TOTAL-1); // 2GB split among compnodes
	// size_t mem_available = 0x2000000 / (KERNEL_NUM_TOTAL-1); // 2GB split among compnodes
	
	// size_t bsize_with_aura = ((X/xnodes)+2) * ((Y/ynodes)+2) * sizeof(unsigned int);
	// size_t bsize_pow2_aligned;

	// if (ispowerof2(bsize_with_aura))
	// 	bsize_pow2_aligned = bsize_with_aura;
	// else
	// 	bsize_pow2_aligned = powerof2(log2floor(bsize_with_aura)+1);

    // mem_available=bsize_pow2_aligned*2;
	size_t mem_available = SEGMENT_SIZE;
	size_t ABoffset = mem_available/2;
    PRINT("Allocating %zu bytes\n", mem_available);
	// GASNET SHARED MEM ALLOCATION AND ATTACH
	// if (myid == ctrlnode) // 0 shared memory
	// 	gasnet_attach(handlers, sizeof(handlers)/sizeof(gasnet_handlerentry_t), (uintptr_t)(uint64_t)0, (uintptr_t)0);
	// else
	// 	gasnet_attach(handlers, sizeof(handlers)/sizeof(gasnet_handlerentry_t), (uintptr_t)(uint64_t)(mem_available), (uintptr_t)0);
	#ifndef __HLS__
    kernel.attach(nullptr, 0, SEGMENT_SIZE);
    #endif
	
	// GET GASNET SEGMENT INFO
	// #ifdef __MICROBLAZE__
	// 	gasnet_seginfo_t segment_table_app[GASNET_NUM_NODES];
	// #else // x86-64, ARM
	// 	gasnet_seginfo_t *segment_table_app;
	// 	segment_table_app = malloc(gasnet_nodes() * sizeof(gasnet_seginfo_t));
	// #endif
	// gasnet_getSegmentInfo(segment_table_app,gasnet_nodes());

	// determine shared block addresses
	// mysharedmem = (void*)(uint64_t)segment_table_app[myid].addr;
	#ifdef __HLS__
	mysharedmem = (char*) local_mem;
	#else
	mysharedmem = (char*) gasnet_shared_mem;
	#endif

	
	///////////////////////////////////
	// SIZE AND NEIGHBOUR INFORMATION
	///////////////////////////////////
	
	unsigned int xdiv = X / xnodes;
	unsigned int ydiv = Y / ynodes;
	ptrdiff_t yoffset = xdiv + 2;
	
	CELLTYPE *memBlockA, *memBlockB, *localA, *localB;
		
	//local node properties
	unsigned int myX,myY,myXmin,myXmax,myYmin,myYmax;
	
	// determine neighbour node information (2D-Torus)
	unsigned int nodeN,nodeE,nodeS,nodeW;
	// send to neighbours? -- special cases for edges
	unsigned int sendN,sendS,sendW,sendE;

	// determine neighbour cell information
	ptrdiff_t celloffsetN,celloffsetE,celloffsetS,celloffsetW;

	// get neighbour memory localities
	voidp64 remoteMemN,remoteMemE,remoteMemS,remoteMemW;
	voidp64    localMemN,localMemE,localMemS,localMemW;
	
	node_jacobidata_str *njdata;
	
	
	///////////////////////////////////
	// MEMORY AND PARAMETER INIT
	///////////////////////////////////

	// clear data fields
	#ifdef __HLS__
	start_timer(axi_timer);
	#else
	auto timer = get_time();
	time_point_t timer_init = timer;

	if (!(myid == ctrlnode))
		memset(mysharedmem,0,mem_available);
	#endif
	
	node_jacobidata_str njdata_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
	uint64_t *stats_squarederror;
	uint64_t stats_squarederror_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
	uint64_t *stats_gradient;
	uint64_t stats_gradient_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
	if (myid == ctrlnode)
	{
		
		njdata = njdata_master;
		// njdata = (node_jacobidata_str*) aligned_alloc(sizeof(word_t), (KERNEL_NUM_TOTAL-1)*sizeof(node_jacobidata_str));
		// std::cout << "njdata at " << std::hex << njdata << std::dec << "\n";
		
		// stats_squarederror = (uint64_t *)aligned_alloc(sizeof(word_t), (KERNEL_NUM_TOTAL-1)*sizeof(uint64_t));
		// stats_gradient = (uint64_t *)aligned_alloc(sizeof(word_t), (KERNEL_NUM_TOTAL-1)*sizeof(uint64_t));
		// uint64_t stats_squarederror_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
		// uint64_t stats_gradient_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
		stats_squarederror = stats_squarederror_master;
		stats_gradient = stats_gradient_master;
		COMPILER_BARRIER

		for(u=0 ; u<(KERNEL_NUM_TOTAL-1) ; u++)
		{
			//compnode properties
			myX = (u) % xnodes;
			myY = (u) / xnodes;
			myXmin = myX * xdiv;
			myXmax = (myX+1)*xdiv - 1;
			myYmin = myY * ydiv;
			myYmax = (myY+1)*ydiv - 1;

			njdata[u].xsize = xdiv; njdata[u].ysize = ydiv;
			njdata[u].pole1xy = 0xFFFFFFFF; //pre-initialize invalid
			njdata[u].pole2xy = 0xFFFFFFFF; //pre-initialize invalid
			njdata[u].pole3xy = 0xFFFFFFFF; //pre-initialize invalid
			njdata[u].pole4xy = 0xFFFFFFFF; //pre-initialize invalid
			njdata[u].pole4value = 0; // initialize to 0
			int itr = 0;
			for(v=0;v<MAXPOLES;v++)
				
			{
				if ( (poles[v].valid) &&
						(poles[v].xpos >= myXmin) && (poles[v].xpos <= myXmax) && 
						(poles[v].ypos >= myYmin) && (poles[v].ypos <= myYmax) ) 
				{
					if      (itr==0) { njdata[u].pole1value = poles[v].charge; njdata[u].pole1xy = ((poles[v].xpos-myXmin) << 16) | (poles[v].ypos-myYmin); }
					else if (itr==1) { njdata[u].pole2value = poles[v].charge; njdata[u].pole2xy = ((poles[v].xpos-myXmin) << 16) | (poles[v].ypos-myYmin); }
					else if (itr==2) { njdata[u].pole3value = poles[v].charge; njdata[u].pole3xy = ((poles[v].xpos-myXmin) << 16) | (poles[v].ypos-myYmin); }
					else if (itr==3) { njdata[u].pole4value = poles[v].charge; njdata[u].pole4xy = ((poles[v].xpos-myXmin) << 16) | (poles[v].ypos-myYmin); }
					itr++;
				}
			}

			// PRINT("xsize: %d\n", njdata[u].xsize);
			// PRINT("ysize: %d\n", njdata[u].ysize);
			// PRINT("pole1xy: %d\n", njdata[u].pole1xy);
			// PRINT("pole1value: %d\n", njdata[u].pole1value);
			// PRINT("pole2xy: %d\n", njdata[u].pole2xy);
			// PRINT("pole2value: %d\n", njdata[u].pole2value);
			// PRINT("pole3xy: %d\n", njdata[u].pole3xy);
			// PRINT("pole3value: %d\n", njdata[u].pole3value);
			// PRINT("pole4xy: %d\n", njdata[u].pole4xy);
			// PRINT("pole4value: %d\n", njdata[u].pole4value);
		}
	}
	else // compnodes
	{
		memBlockA = (CELLTYPE*)mysharedmem;
		memBlockB = (CELLTYPE*)(mysharedmem + ABoffset);
		localA = memBlockA + (yoffset+1);
		localB = memBlockB + (yoffset+1);
			
		//local node properties
		myX = (my_compnode) % xnodes;
		myY = (my_compnode) / xnodes;
		myXmin = myX * xdiv;
		myXmax = (myX+1)*xdiv - 1;
		myYmin = myY * ydiv;
		myYmax = (myY+1)*ydiv - 1;
		
		// determine neighbour node information (2D-Torus)
		nodeN  = compnode0 + ((myY+ynodes-1)%ynodes) * xnodes + ((myX+xnodes+0)%xnodes);
		nodeE  = compnode0 + ((myY+ynodes+0)%ynodes) * xnodes + ((myX+xnodes+1)%xnodes);
		nodeS  = compnode0 + ((myY+ynodes+1)%ynodes) * xnodes + ((myX+xnodes+0)%xnodes);
		nodeW  = compnode0 + ((myY+ynodes+0)%ynodes) * xnodes + ((myX+xnodes-1)%xnodes);
		// send to neighbours? -- special cases for edges
		sendN = (myY!=0);
		sendS = (myY!=(ynodes-1));
		sendW = (myX!=0);
		sendE = (myX!=(xnodes-1));

		// determine neighbour cell information
		celloffsetN  = +0-yoffset;
		celloffsetE  = +1;
		celloffsetS  = -0+yoffset;
		celloffsetW  = -1;

		// get neighbour memory localities
		// remoteMemN  = (voidp64)(uint64_t)kernel.get_segment_addr(nodeN) + ((1     ) + (ydiv+0)*yoffset) * sizeof(CELLTYPE);
		// remoteMemE  = (voidp64)(uint64_t)kernel.get_segment_addr(nodeE) + ((1     ) + (    +1)*yoffset) * sizeof(CELLTYPE);
		// remoteMemS  = (voidp64)(uint64_t)kernel.get_segment_addr(nodeS) + ((1     ) + (    +1)*yoffset) * sizeof(CELLTYPE);
		// remoteMemW  = (voidp64)(uint64_t)kernel.get_segment_addr(nodeW) + ((0+xdiv) + (    +1)*yoffset) * sizeof(CELLTYPE);
		remoteMemN  = ((1     ) + (ydiv+0)*yoffset) * sizeof(CELLTYPE);
		remoteMemE  = ((1     ) + (    +1)*yoffset) * sizeof(CELLTYPE);
		remoteMemS  = ((1     ) + (    +1)*yoffset) * sizeof(CELLTYPE);
		remoteMemW  = ((0+xdiv) + (    +1)*yoffset) * sizeof(CELLTYPE);

		localMemN  = (voidp64)(((1     ) + (    +0)*yoffset) * sizeof(CELLTYPE));
		localMemE  = (voidp64)(((1+xdiv) + (    +1)*yoffset) * sizeof(CELLTYPE));
		localMemS  = (voidp64)(((1     ) + (ydiv+1)*yoffset) * sizeof(CELLTYPE));
		localMemW  = (voidp64)(((0     ) + (    +1)*yoffset) * sizeof(CELLTYPE));
		
		// determine expected neighbour bytes
		// shared->expected_neighbour_bytes = (xdiv*sendN + xdiv*sendS + ydiv*sendW + ydiv*sendE) * sizeof(CELLTYPE);
	}

	node_jacobidata_str njdata_local;
	
	// barrier so that memory is cleared before messages start coming in; collecting node is 0
	// thegasnet_sharedMemoryReady(ctrlnode);
	if(myid == ctrlnode){
		#pragma HLS INLINE
		kernel.barrier_wait();
	} else {
		#pragma HLS INLINE
		kernel.barrier_send(ctrlnode);
	}
	
	// Init time
	// Timer_TakeTime();
	#ifdef __HLS__
	word_t time;
	if(PRINTCOND){
		time = get_time(axi_timer);
		send_time(&kernel, 0x0, time, axi_timer);
	}
	#else
	auto memory_time = get_time_diff(timer_init);
	if(PRINTCOND){
		std::cout << "Memory init time: " << memory_time << " s\n";
	}
	#endif
	// PRINTTIME(PRINTCOND,"Memory init time:");

	PRINT("Node %d ready.\r\n",myid);
	// barrier(ctrlnode);
	if(myid == ctrlnode){
		kernel.barrier_wait();
	} else {
		kernel.barrier_send(ctrlnode);
	}
	if (PRINTCOND)
	{
		PRINT("Node\tIteration\tSum of sqdiffs\tLargest gradient\r\n");
	}	
	// Timer_Start();
	#ifdef __HLS__
	start_timer(axi_timer);
	#else
	timer = get_time();
	#endif

#define ITERATIONS 3
	
	CELLTYPE *dstBlock,*srcBlock;
	CELLTYPE cellsum;
	int print_thr = 1;

	// PARAMETER DISTRIBUTION / RECEPTION
	if (myid == ctrlnode)
	{
		// std::cout << "Sending jacobi data struct\n";
		for(u=0 ; u<(KERNEL_NUM_TOTAL-1) ; u++){
			// gasnet_AMRequestMedium0(u,hc_M0_compnode_parameters,&(njdata[u]),sizeof(node_jacobidata_str));
			kernel.sendMediumAM_async(u, 0xabc, 0, 0, nullptr, sizeof(node_jacobidata_str));
			#ifdef __HLS__
			word_t payload;
			payload = ((word_t)(njdata_master[u].ysize) << 32) | njdata_master[u].xsize;
			kernel.sendPayload(u, payload, false);
			payload = ((word_t)(njdata_master[u].pole1value) << 32) | njdata_master[u].pole1xy;
			kernel.sendPayload(u, payload, false);
			payload = ((word_t)(njdata_master[u].pole2value) << 32) | njdata_master[u].pole2xy;
			kernel.sendPayload(u, payload, false);
			payload = ((word_t)(njdata_master[u].pole3value) << 32) | njdata_master[u].pole3xy;
			kernel.sendPayload(u, payload, false);
			payload = ((word_t)(njdata_master[u].pole4value) << 32) | njdata_master[u].pole4xy;
			kernel.sendPayload(u, payload, true);		
			#else
			for(int i = 0; i < sizeof(node_jacobidata_str)/GC_DATA_BYTES; i++){
				kernel.sendPayload(u, (((word_t*)(&(njdata[u])))[i]), i == (sizeof(node_jacobidata_str)/GC_DATA_BYTES-1));
				// std::cout << i << ": " << std::hex << &(((word_t*)(&(njdata[u])))[i]) << std::dec << "\n";
				// std::cout << "data: " << std::hex << (((word_t*)(&(njdata[u])))[i]) << std::dec << "\n";
			}
			#endif
			
		}
		// free(njdata);
		
	}
	else // compnodes
	{
		// while(!READ_SHARED(shared->njdata_local_received))
		// {
		// 	sched_yield();
		// }
		// COMPILER_BARRIER;
		// shared->njdata_local_received = 0;
		size_t _size;
		short _dest, _id;
		// in->packet_read((((char*)(&(njdata_local)))), &_size, &_dest, &_id);
		#ifdef __HLS__
			galapagos::stream_packet <word_t> axis_word[sizeof(node_jacobidata_str)/GC_DATA_BYTES];
			// word_t* njdata_local_ptr = (word_t*)(&njdata_local);
			in->read(); // read token
			for(int q = 0; q < sizeof(node_jacobidata_str)/GC_DATA_BYTES; q++){
				in->read(axis_word[q]);
			}
			njdata_local.xsize = axis_word[0].data & 0xFFFFFFFF;
			njdata_local.ysize = ((axis_word[0].data) >> 32) & 0xFFFFFFFF;
			njdata_local.pole1xy = axis_word[1].data & 0xFFFFFFFF;
			njdata_local.pole1value = ((axis_word[1].data) >> 32) & 0xFFFFFFFF;
			njdata_local.pole2xy = axis_word[2].data & 0xFFFFFFFF;
			njdata_local.pole2value = ((axis_word[2].data) >> 32) & 0xFFFFFFFF;
			njdata_local.pole3xy = axis_word[3].data & 0xFFFFFFFF;
			njdata_local.pole3value = ((axis_word[3].data) >> 32) & 0xFFFFFFFF;
			njdata_local.pole4xy = axis_word[4].data & 0xFFFFFFFF;
			njdata_local.pole4value = ((axis_word[4].data) >> 32) & 0xFFFFFFFF;
		#else
		word_t* njdata_local_ptr = (word_t*)(in->packet_read(&_size, &_dest, &_id));
		// skip one for the header
		memcpy(&njdata_local, &(njdata_local_ptr[1]), sizeof(node_jacobidata_str));

		free(njdata_local_ptr);
		#endif
		
		//PRINT("Node %d\tXsize %d\tYsize %d\tPole1X %d\tPole1Y %d\tValue1 %d\tPole2X %d\tPole2Y %d\tValue2 %d\r\n",myid,shared->njdata_local.xsize,shared->njdata_local.ysize,(shared->njdata_local.pole1xy>>16),(shared->njdata_local.pole1xy&0xFFFF),shared->njdata_local.pole1value,(shared->njdata_local.pole2xy>>16),(shared->njdata_local.pole2xy&0xFFFF),shared->njdata_local.pole2value);
	}
	
	
	///////////////////////////////////
	// MAIN LOOP
	///////////////////////////////////
	
	for(t=0;t<ITERATIONS;t++)
	{
		if (myid == ctrlnode)
		{
			// MESSAGING PHASE //
			// SPINLOCK FOR COMPLETE MESSAGE RECEIVE //

			// barrier(ctrlnode);///
			kernel.barrier_wait();
			
			// COMPUTATION PHASE //
			
			// STATS RECEPTION / BARRIER
			// while(!(READ_SHARED(shared->stats_cnt) == (KERNEL_NUM_TOTAL-1))) sched_yield();
			// shared->stats_cnt = 0;
			kernel.wait_counter(KERNEL_NUM_TOTAL-1);
			// COMPILER_BARRIER

			for(int q = 0; q < (KERNEL_NUM_TOTAL-1); q++){
				#ifdef __HLS__
				galapagos::stream_packet <word_t> axis_word;
				axis_word = in->read();
				word_t token = hdextract(axis_word.data, AM_SRC);
				axis_word = in->read();
				stats_squarederror_master[token] = axis_word.data;
				axis_word = in->read();
				stats_gradient_master[token] = axis_word.data;
				#else
				word_t token = hdextract(in->read().data, AM_SRC);
				stats_squarederror[token] = in->read().data;
				stats_gradient[token] = in->read().data;
				#endif
				
			}
			COMPILER_BARRIER
		
            // PRINT("ITERATION: %04d\n", t);
	
			if (t % print_thr==0)  
			{
				// PRINTTIME(PRINTCOND,"Time:");
				#ifdef __HLS__
				if(PRINTCOND){
					time = get_time(axi_timer);
					send_time(&kernel, 0x1, time, axi_timer);
				}
				#else
				auto memory_time = get_time_diff(timer_init);
				if(PRINTCOND){
					std::cout << "Time: " << memory_time << " s\n";
				}
				#endif
				for(u=0 ; u<(KERNEL_NUM_TOTAL-1) ; u++)
				{
					//if ((stats_squarederror[u]) || (stats_gradient[u]))
						PRINT("Node %02d stats: \tIteration %04d\t%lld\t%d\r\n",u,t,stats_squarederror[u],stats_gradient[u]);
				}
				PRINT("7777\t------------------------------------------------------------------------------------------\r\n");
				print_thr <<= 1;
			}
		
            //while(1);	
			// if (t<ITERATIONS)
			// {
			// 	for(u=0 ; u<(KERNEL_NUM_TOTAL-1) ; u++)
			// 		gasnet_AMRequestShort1(u,hc_S1_stats_reply,1);
			// }
			// else
			// {
			// 	for(u=0 ; u<(KERNEL_NUM_TOTAL-1) ; u++)
			// 		gasnet_AMRequestShort1(u,hc_S1_stats_reply,-1);
			// 	break; // leave iteration loop
			// }
			kernel.barrier_wait();
			
		}
		else // IS COMPNODE
		{
			// shared->continue_running = 0;
			
			// block determination
			unsigned int odd = t % 2;
			unsigned int even = !odd;

			/////////////////////
			// MESSAGING PHASE //
			/////////////////////
			
			ptrdiff_t yoffset = njdata_local.xsize + 2;

			// sleep(1);

		    // if (sendN) gasnet_get_nbi(localMemN +(odd*ABoffset), nodeN, remoteMemN +(odd*ABoffset), njdata_local.xsize*sizeof(CELLTYPE));
			if (sendN){
				kernel.getLongAM_normal(nodeN, 0x01, H_INCR_MEM, 0, nullptr, njdata_local.xsize*sizeof(CELLTYPE), (word_t)(remoteMemN +(odd*ABoffset)), (word_t)(localMemN +(odd*ABoffset)));
				kernel.wait_mem(1);
			}
		    // if (sendS) gasnet_get_nbi(localMemS +(odd*ABoffset), nodeS, remoteMemS +(odd*ABoffset), njdata_local.xsize*sizeof(CELLTYPE));
			if (sendS){
				kernel.getLongAM_normal(nodeS, 0x02, H_INCR_MEM, 0, nullptr, njdata_local.xsize*sizeof(CELLTYPE), (word_t)(remoteMemS +(odd*ABoffset)), (word_t)(localMemS +(odd*ABoffset)));
				kernel.wait_mem(1);
			}
            // if (sendW) gasnet_gets_nbi(localMemW+(odd*ABoffset), nodeW, remoteMemW+(odd*ABoffset), njdata_local.ysize*sizeof(CELLTYPE), 
            //                                         yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), njdata_local.ysize,
            //                                         yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), njdata_local.ysize);
			if (sendW){
				kernel.getLongStrideAM_normal(nodeW, 0x03, H_INCR_MEM, 0, nullptr, njdata_local.ysize*sizeof(CELLTYPE), 
					yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), njdata_local.ysize, (word_t)(remoteMemW+(odd*ABoffset)),
					yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), njdata_local.ysize, (word_t)(localMemW+(odd*ABoffset))
				);
				kernel.wait_mem(1);
			}
            // if (sendE) gasnet_gets_nbi(localMemE+(odd*ABoffset), nodeE, remoteMemE+(odd*ABoffset), njdata_local.ysize*sizeof(CELLTYPE), 
            //                                         yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), njdata_local.ysize, 
            //                                         yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), njdata_local.ysize);
			if (sendE){
				kernel.getLongStrideAM_normal(nodeE, 0x04, H_INCR_MEM, 0, nullptr, njdata_local.ysize*sizeof(CELLTYPE), 
					yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), njdata_local.ysize, (word_t)(remoteMemE+(odd*ABoffset)),
					yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), njdata_local.ysize, (word_t)(localMemE+(odd*ABoffset))
				);
				kernel.wait_mem(1);
				// for(int p = 0; p < njdata_local.ysize; p++){
				// 	char* tmp = (char*)(gasnet_shared_mem + (word_t)(localMemE+(odd*ABoffset)+(p*yoffset*sizeof(CELLTYPE))));
				// 	CELLTYPE tmp_val = (CELLTYPE) *(tmp);
				// 	if (tmp_val != 0){
				// 		PRINT("Data %p %lld %d\n", tmp, (CELLTYPE) *tmp, p);
				// 	}
					
				// }
			}

			// sleep(1);

			///////////////////////////////////////////
			// SPINLOCK FOR COMPLETE MESSAGE RECEIVE //
			///////////////////////////////////////////
			if (even)
			{
				srcBlock = localA; dstBlock = localB;
			}
			else //odd
			{
				srcBlock = localB; dstBlock = localA; 
			}

			// waiting for incoming data
			
		    //gasnet_wait_syncnbi_all();
			
			// MESSAGING TIME
	#ifdef INTERMEDIATE_TIMES
			PRINTACCTIME(TIMECOND,"Communication time:",commsecs,commusecs);
	#else
			// ACCTIME(TIMECOND,commsecs,commusecs);
			if(TIMECOND){
				#ifdef __HLS__
				commtime = accum_time(commtime, axi_timer);
				start_timer(axi_timer);
				#else
				commtime = accum_time(timer, commtime);
				timer = get_time();
				#endif
			}
	#endif
			// barrier(ctrlnode);///
			kernel.barrier_send(ctrlnode);
	#ifdef INTERMEDIATE_TIMES
			PRINTACCTIME(TIMECOND,"Barrier time:",bar1secs,bar1usecs);
	#else
		if(TIMECOND){
			#ifdef __HLS__
			bar1time = accum_time(bar1time, axi_timer);
			start_timer(axi_timer);
			#else
			// ACCTIME(TIMECOND,bar1secs,bar1usecs);
			bar1time = accum_time(timer, bar1time);
			timer = get_time();
			#endif
		}
	#endif
			int32_t diff = 0;
			uint64_t diffsum = 0;
			uint64_t diffsquaresum = 0;
			CELLTYPE sourceN = 0;
			CELLTYPE sourceE = 0;
			CELLTYPE sourceS = 0;
			CELLTYPE sourceW = 0;
			CELLTYPE sourceC = 0;
			CELLTYPE gradN = 0;
			CELLTYPE gradE = 0;
			CELLTYPE gradS = 0;
			CELLTYPE gradW = 0;
			CELLTYPE maxgrad = 0;
			///////////////////////
			// COMPUTATION PHASE //
			///////////////////////
			for(y = 0; y < njdata_local.ysize; y++)
			{
				
				for(x = 0; x < njdata_local.xsize; x++) // per cell operations
				{
					// JACOBI relaxation
					
					sourceN = *(srcBlock+celloffsetN );
					sourceW = *(srcBlock+celloffsetW );
					sourceE = *(srcBlock+celloffsetE );
					sourceS = *(srcBlock+celloffsetS );
					sourceC = *srcBlock;
					
					gradN = abs(sourceN - sourceC);
					if (gradN>maxgrad) maxgrad = gradN;
					gradS = abs(sourceS - sourceC);
					if (gradS>maxgrad) maxgrad = gradS;
					gradW = abs(sourceW - sourceC);
					if (gradW>maxgrad) maxgrad = gradW;
					gradE = abs(sourceE - sourceC);
					if (gradE>maxgrad) maxgrad = gradE;
					
					cellsum =	sourceN + sourceW + sourceE + sourceS;
					
					cellsum /= 4;
					//uint32_t cellsum_bak = cellsum;
					
					// static poles
					if (njdata_local.pole1xy == ((x<<16) | y))
						cellsum += njdata_local.pole1value;
					if (njdata_local.pole2xy == ((x<<16) | y))
						cellsum += njdata_local.pole2value;
					if (njdata_local.pole3xy == ((x<<16) | y))
						cellsum += njdata_local.pole3value;
					if (njdata_local.pole4xy == ((x<<16) | y))
						cellsum += njdata_local.pole4value;
						
					//if((cellsum) && (myid==15) && (t<10))
						//PRINT("X%d Y%d - cell w/o pole: %d - w/ pole: %d - A2:%d,B1:%d,B3:%d,C2:%d\r\n",x,y,cellsum_bak,cellsum,sourceN,sourceW,sourceE,sourceS);
                   
					*dstBlock = cellsum;

					diff = *srcBlock-*dstBlock; // used for average later

					// if(diff != 0){
					// 	PRINT("diff is %lld at (%d, %d)\n", diff, x, y);
					// }
					
					//diffsqare = ;
					diffsquaresum += (int64_t)diff*(int64_t)diff;
					
					srcBlock++;
					dstBlock++;
				}
				// jump over aura cells
				srcBlock += 2;
				dstBlock += 2;
			}
			
           // PRINT("Node\t%d\tCellSum\t%d\r\n", myid, cellsum); 
/*			if ((t)%print_thr==0)
			{
				//diffsquaresumroot = sqrt(diffsquaresum);
				//PRINT("%d\t%d\t%lld\t%lld\t%d\t%d\r\n",myid,t, \
				//		diffsquaresum,diffsquaresumroot,diffsquaresum/(xdiv*ydiv),maxgrad);
				//PRINT("%d\t%d\t%llX\t%d\r\n",myid,t,diffsquaresum,maxgrad);
			}*/
			
			//////////////////////////////
			// STATS RECEPTION / BARRIER
			//////////////////////////////

			// COMPUTATION TIMING
			#ifdef INTERMEDIATE_TIMES
					PRINTACCTIME(TIMECOND,"Computation time:",compsecs,compusecs);
			#else
				if(TIMECOND){
					#ifdef __HLS__
					comptime = accum_time(comptime, axi_timer);
					start_timer(axi_timer);
					#else
					// ACCTIME(TIMECOND,compsecs,compusecs)
					comptime = accum_time(timer, comptime);
					timer = get_time();
					#endif
				}
					
			#endif
	    
			// gasnet_AMRequestShort4(ctrlnode, hc_S4_send_stats, myid, (uint32_t)(diffsquaresum & 0xFFFFFFFF), (uint32_t)(diffsquaresum>>32), maxgrad);
			// while(READ_SHARED(shared->continue_running)==0) sched_yield();
			const word_t increment = 1;
			#ifdef __HLS__
			kernel.sendMediumAM_async(ctrlnode, myid, H_ADD, 1, 2);
			kernel.sendPayload(ctrlnode, increment, false);
			#else
			kernel.sendMediumAM_async(ctrlnode, myid, H_ADD, 1, &increment, 2);
			#endif
			// std::cout << "diffsquaresum is " << diffsquaresum << "\n";
			kernel.sendPayload(ctrlnode, diffsquaresum, false);
			// std::cout << "maxgrad is " << maxgrad << "\n";
			kernel.sendPayload(ctrlnode, maxgrad, true);

			kernel.barrier_send(ctrlnode);
			COMPILER_BARRIER
		
			
			#ifdef INTERMEDIATE_TIMES
					PRINTACCTIME(TIMECOND,"Barrier time:",bar2secs,bar2usecs);
			#else
				if(TIMECOND){
					#ifdef __HLS__
					bar2time = accum_time(bar2time, axi_timer);
					start_timer(axi_timer);
					#else
					// ACCTIME(TIMECOND,bar2secs,bar2usecs)
					bar2time = accum_time(timer, bar2time);
					timer = get_time();
					#endif
				}
			#endif
		
			// if (shared->continue_running==-1) break; // leave iteration loop
			
		} // ELSE (IS COMPNODE)

	} // ITERATION LOOP
	
	
	if (TIMECOND){
		#ifdef __HLS__
		send_time(&kernel, 0x2, commtime, axi_timer);
		send_time(&kernel, 0x3, bar1time, axi_timer);
		send_time(&kernel, 0x4, comptime, axi_timer);
		send_time(&kernel, 0x5, bar2time, axi_timer);
		#else
		// char TIMEOUTPUT[100];
		// sprintf(TIMEOUTPUT,"Cumulative communication time: %d.%06ds\r\n",commsecs,commusecs); PRINT("%s",TIMEOUTPUT);
		// sprintf(TIMEOUTPUT,"Cumulative barrier 1 time    : %d.%06ds\r\n",bar1secs,bar1usecs); PRINT("%s",TIMEOUTPUT);
		// sprintf(TIMEOUTPUT,"Cumulative computation time  : %d.%06ds\r\n",compsecs,compusecs); PRINT("%s",TIMEOUTPUT);
		// sprintf(TIMEOUTPUT,"Cumulative barrier 2 time    : %d.%06ds\r\n",bar2secs,bar2usecs); PRINT("%s",TIMEOUTPUT);
		std::cout << "Cumulative communication time: " << commtime << "\n";
		std::cout << "Cumulative barrier 1 time: " << bar1time << "\n";
		std::cout << "Cumulative computation time: " << comptime << "\n";
		std::cout << "Cumulative barrier time: " << bar2time << "\n";
		//	sprintf(TIMEOUTPUT,"Cumulative printout time: %d.%06ds\r\n",dispsecs,dispusecs); PRINT("%s",TIMEOUTPUT);
		#endif
	}

	/// TEMPORARY EXIT SOLUTION
	// barrier(ctrlnode);
	if(myid == ctrlnode){
		#ifdef __HLS__
		// read sent times out but do nothing
		for(int i = 0; i < 8; i++){
			in->read();
		}
		#endif
		kernel.barrier_wait();
	} else {
		kernel.barrier_send(ctrlnode);
	}
	PRINT("Node %d done.\r\n",myid); 
	
	// exit cleanly
	//gasnet_exit(42);
	// free(stats_squarederror);
	// free(stats_gradient);

	kernel.end();

	#ifdef __HLS__
	// dummy write to convey that we got to this point
	local_mem[0xab] = 0xcd;
	#endif
	
	// return 0;
}

#ifndef __HLS__

void kern0(
    short id,
    galapagos::interface <word_t> * in,
    galapagos::interface<word_t> * out
){
    jacobi(id, in, out);
}

PGAS_METHOD(kern0, 0);
#endif
// PGAS_METHOD(kern1, KERN1_ID);
// PGAS_METHOD(kern2, KERN2_ID);
// PGAS_METHOD(kern3, KERN3_ID);
// PGAS_METHOD(kern4, KERN4_ID);
// void __real_kern0(short id, galapagos::interface <word_t> *in, galapagos::interface <word_t> *out);
// void __wrap_kern0 (short id, galapagos::interface <word_t> *in, galapagos::interface <word_t> *out){
// 	void (*fcnPtr)(short id, galapagos::interface <word_t> *, galapagos::interface <word_t> *) = __real_kern0;
// 	handler_thread(fcnPtr, 0, in, out);
// }
}


// void S0_barrier_increment(gasnet_token_t token)
// {
// 	USE_NODE_GLOBALS(shared);
// //		COMPILER_BARRIER
// 	shared->barrier_cnt++;
// //		COMPILER_BARRIER
// 	return;
// }

// void L0_update_aura(gasnet_token_t token, void *buf, size_t nbytes)
// {
// 	USE_NODE_GLOBALS(shared);

// #ifdef __x86_64__ARM__
// 	//if (!arg0) // even
// 	{
// 		size_t received;
// 		received = __sync_add_and_fetch(&(shared->bytes_received), nbytes);
// 		if (received == expected_neighbour_bytes)
// 		{
// 			__sync_fetch_and_and(&(shared->bytes_received), 0); // clear
// 			__sync_fetch_and_or(&(shared->ready),1);
// 		}
// 	}
// 	/*else // odd
// 	{
// 		size_t received;
// 		received = __sync_add_and_fetch(&(shared->bytes_receivedA), nbytes);
// 		if (received == expected_neighbour_bytes)
// 		{
// 			__sync_fetch_and_and(&(shared->bytes_receivedA), 0); // clear
// 			__sync_fetch_and_or(&(shared->readyA),1);
// 		}
// 	}*/
// #else // __MICROBLAZE__
// 	if (!arg0) // even
// 	{
// 		size_t received;
// 		COMPILER_BARRIER
// 			received = (shared->bytes_receivedB += nbytes);
// 		COMPILER_BARRIER

// 		if (received == expected_neighbour_bytes)
// 		{
// 		COMPILER_BARRIER
// 			shared->bytes_receivedB = 0; // clear
// 		COMPILER_BARRIER
// 			shared->readyB = 1;
// 		COMPILER_BARRIER
// 		}
// 	}
// 	else // odd
// 	{
// 		size_t received;
// 		COMPILER_BARRIER
// 			received = (shared->bytes_receivedA += nbytes);
// 		COMPILER_BARRIER
		
// 		if (received == expected_neighbour_bytes)
// 		{
// 			COMPILER_BARRIER
// 				shared->bytes_receivedA = 0; // clear
// 			COMPILER_BARRIER
// 				shared->readyA = 1;
// 			COMPILER_BARRIER
// 		}
// 	}
// #endif
	
// 	return;
// }

// void M0_compnode_parameters(gasnet_token_t token, void *buf, size_t nbytes)
// {
// 	USE_NODE_GLOBALS(shared);
	
// 	memcpy((void*)&(shared->njdata_local),buf,sizeof(node_jacobidata_str));
// 	shared->njdata_local_received = 1;
// }

// void S4_send_stats(gasnet_token_t token, gasnet_handlerarg_t arg0, gasnet_handlerarg_t arg1, gasnet_handlerarg_t arg2, gasnet_handlerarg_t arg3)
// {
// 	USE_NODE_GLOBALS(shared);
	
// 	stats_squarederror[arg0] = (((uint64_t)arg2)<<32) | (uint32_t)arg1;
// 	stats_gradient[arg0] = arg3;
// 	COMPILER_BARRIER
// 	shared->stats_cnt++;
// }

// void S1_stats_reply(gasnet_token_t token, gasnet_handlerarg_t arg0)
// {
// 	USE_NODE_GLOBALS(shared);
	
// 	shared->continue_running = arg0;
// }

// void barrier(unsigned int ctrlnode)
// {
// 	USE_NODE_GLOBALS(shared); 
	
// 	unsigned int t;

// 	if (myid==ctrlnode)
// 	{
// 		while(READ_SHARED(shared->barrier_cnt)!=gasnet_nodes()-1) sched_yield();
// 		COMPILER_BARRIER
// 		shared->barrier_cnt=0;
// 		COMPILER_BARRIER
// 		for(t=0;t<gasnet_nodes();t++)
// 			if (t != ctrlnode)
// 				gasnet_AMRequestShort0(t, hc_S0_barrier_increment);
// 	}
// 	else
// 	{
// 		// Send message to ctrl node
// 		gasnet_AMRequestShort0(ctrlnode, hc_S0_barrier_increment);
// 		while(READ_SHARED(shared->barrier_cnt)==0) sched_yield();
// 		COMPILER_BARRIER
// 		shared->barrier_cnt=0;
// 		COMPILER_BARRIER
// 	}
// }

