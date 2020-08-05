#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <math.h>

#include "jacobi.hpp"

#undef  INTERMEDIATE_TIMES

#define CELLTYPE int64_t

#ifdef __HLS__
// this function, if inlined, or placed in code, behaves badly in Verilog. In
// simulation, it does do three writes but they appear as burst writes to the wrong
// addresses. Doing it this way seems to work properly. Tested in HLS 2018.1
void start_timer_ctrl(volatile int* axi_timer){
    axi_timer[0] = 0x0; // stop timer
    axi_timer[1] = 0; // set load register to 0
    axi_timer[0] = 0x20; // load timer with load register
    axi_timer[0] = 0x80; // start timer
}

word_t get_time_ctrl(volatile int* axi_timer){
    // #pragma HLS INLINE
    // axi_timer[0] = 0x0; // stop timer
    word_t time = *(axi_timer + 0x2); // read timer count
	return time;
}

word_t accum_time_ctrl(word_t old_time, volatile int* axi_timer){
	word_t time = get_time_ctrl(axi_timer);
	return old_time + time;
}

void send_time_ctrl(shoal::kernel* kernel, gc_AMToken_t token, word_t time, volatile int* axi_timer){
	#pragma HLS INLINE
    word_t dumb = *(axi_timer);
    if(dumb != 0xFF){
    kernel->sendMediumAM_async(KERNEL_NUM_TOTAL-1, token, H_EMPTY, 0, NULL, 8);
    kernel->sendPayload(0, time, true); // assume always send to kernel 0
    }
    // axi_timer[0] = 0x0; // stop timer
    // kernel->wait_reply(1, axi_timer);
}
#else
auto get_time_ctrl(){
    return std::chrono::high_resolution_clock::now();
}

auto get_time_diff_ctrl(time_point_t timer){
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - timer;
    return elapsed.count();
}

double accum_time_ctrl(time_point_t old_time, double sum_time){
	double delta = get_time_diff_ctrl(old_time);
	double new_sum = sum_time + delta;
	return new_sum;
}
#endif

unsigned int log2floor_ctrl(unsigned int val)
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

unsigned int powerof2_ctrl(unsigned int val)
{
	unsigned int i;
	unsigned int res = 1;
	
	for(i=0 ; i<val ; i++)
		res *= 2;
	
	return res;
}

unsigned int ispowerof2_ctrl(unsigned int val)
{
	if ( powerof2_ctrl(log2floor_ctrl(val)) == val)
		return 1;
	else
		return 0;
}


typedef struct pole{
	int32_t charge;
	unsigned int xpos;
	unsigned int ypos;
	unsigned int valid;
} pole_str;

// right now, this works only because this is 320 bytes or 40 64 bit words even
// it needs to be aligned to the word boundary
typedef struct node_jacobidata{
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

extern "C"{
void jacobi_ctrl(
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
    #pragma HLS INTERFACE m_axi port=local_mem depth=4096 offset=0

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

	#ifdef __HLS__
    shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out, handler_ctrl);
    #else
    shoal::kernel kernel(id, KERNEL_NUM_TOTAL, in, out);
    #endif
	kernel.init();
		
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
	
	/// 2D-Topology: if x!=y then x=2*y  -  TODO other way makes more sense for memory transfer optimization
	unsigned int log2nodes = log2floor_ctrl((KERNEL_NUM_TOTAL-1));
	unsigned int xnodes = (powerof2_ctrl(log2nodes/2 + log2nodes%2));
	unsigned int ynodes = (powerof2_ctrl(log2nodes/2));	
		
	///////////////////////////////////
	// MEMORY ALLOCATION
	///////////////////////////////////
	
	size_t bsize_with_aura = ((X/xnodes)+2) * ((Y/ynodes)+2) * sizeof(CELLTYPE);
	size_t bsize_pow2_aligned;

	if (ispowerof2_ctrl(bsize_with_aura))
		bsize_pow2_aligned = bsize_with_aura;
	else
		bsize_pow2_aligned = powerof2_ctrl(log2floor_ctrl(bsize_with_aura)+1);

    PRINT("Memory required: %zu\n", bsize_pow2_aligned*2);
	size_t mem_available = bsize_pow2_aligned*2;
	size_t ABoffset = mem_available/2;
    PRINT("Allocating %zu bytes\n", mem_available);
	#ifndef __HLS__
    kernel.attach(nullptr, 0, mem_available);
    #endif
	
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
	start_timer_ctrl(axi_timer);
	#else
	auto timer = get_time_ctrl();
	time_point_t timer_init = timer;
	#endif
	
	node_jacobidata_str njdata_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
	uint64_t *stats_squarederror;
	uint64_t stats_squarederror_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
	uint64_t *stats_gradient;
	uint64_t stats_gradient_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
	
		
	njdata = njdata_master;
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
	}
	

	node_jacobidata_str njdata_local;
	
	// kernel.barrier_wait();
	
	#ifdef __HLS__
	word_t time;
	time = get_time_ctrl(axi_timer);
	// send_time_ctrl(&kernel, 0x0, time, axi_timer);
	#else
	auto memory_time = get_time_diff_ctrl(timer_init);
	std::cout << "Grid_size: " << GRID_SIZE << "\n";
	std::cout << "Node_count: " << KERNEL_NUM_TOTAL << "\n";
	std::cout << "Iteration_count: " << ITERATIONS << "\n";
	std::cout << "Nodes: " << NODES	<< "\n";
	std::cout << "Cumulative memory_init time: " << memory_time << "\n";
	#endif

	PRINT("Node %d ready.\r\n",myid);

	kernel.barrier_wait();

	PRINT("Node\tIteration\tSum of sqdiffs\tLargest gradient\r\n");
	// Timer_Start();
	#ifdef __HLS__
	start_timer_ctrl(axi_timer);
	#else
	timer = get_time_ctrl();
	#endif

	CELLTYPE *dstBlock,*srcBlock;
	CELLTYPE cellsum;
	int print_thr = 1;

	// PARAMETER DISTRIBUTION / RECEPTION
	
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
		}
		#endif
		
	}
	
	///////////////////////////////////
	// MAIN LOOP
	///////////////////////////////////
	
	for(t=0;t<ITERATIONS;t++)
	{
		// MESSAGING PHASE //
		// SPINLOCK FOR COMPLETE MESSAGE RECEIVE //

		kernel.barrier_wait();
		
		// COMPUTATION PHASE //
		
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

		if (t % print_thr==0){
			#ifdef __HLS__
			// time = get_time_ctrl(axi_timer);
			// send_time_ctrl(&kernel, 0x1, time, axi_timer);
			#else
			auto memory_time = get_time_diff_ctrl(timer);
			std::cout << "Time: " << memory_time << " s\n";
			#endif
			for(u=0 ; u<(KERNEL_NUM_TOTAL-1) ; u++)
			{
				//if ((stats_squarederror[u]) || (stats_gradient[u]))
					PRINT("Node %02d stats: \tIteration %04d\t%lld\t%lld\r\n",u,t,(long long)stats_squarederror[u],(long long)stats_gradient[u]);
			}
			PRINT("7777\t------------------------------------------------------------------------------------------\r\n");
			print_thr <<= 1;
		}
		kernel.barrier_wait();

	} // ITERATION LOOP
	
	#ifdef __HLS__
	// read sent times out but do nothing
	for(int i = 0; i < 5; i++){
		in->read();
	}
	#else
	galapagos::stream_packet <word_t> axis_word;
	in->read(); //read token
	axis_word = in->read();
	// double time;
	// memcpy(&time, &(axis_word.data), sizeof(double));
	std::cout << "Communication cycles: " << axis_word.data << "\n";
	std::cout << "Cumulative communication time: " << (double) axis_word.data / 1E9 * 6.4 << "\n";
	axis_word = in->read();
	// memcpy(&time, &(axis_word.data), sizeof(double));
	std::cout << "barrier_1 cycles: " << axis_word.data << "\n";
	std::cout << "Cumulative barrier_1 time: " << (double) axis_word.data / 1E9 * 6.4 << "\n";
	axis_word = in->read();
	// memcpy(&time, &(axis_word.data), sizeof(double));
	std::cout << "computation cycles: " << axis_word.data << "\n";
	std::cout << "Cumulative computation time: " << (double) axis_word.data / 1E9 * 6.4 << "\n";
	axis_word = in->read();
	// memcpy(&time, &(axis_word.data), sizeof(double));
	std::cout << "barrier_2 cycles: " << axis_word.data << "\n";
	std::cout << "Cumulative barrier_2 time: " << (double) axis_word.data / 1E9 * 6.4 << "\n";
	#endif
	kernel.barrier_wait();
	
	PRINT("Node %d done.\r\n",myid); 

	kernel.end();

	#ifdef __HLS__
	// dummy write to convey that we got to this point
	local_mem[0xab] = 0xcd;
	while(1){};
	#endif
	
	// return 0;
}

#ifndef __HLS__

void kern1(
    short id,
    galapagos::interface <word_t> * in,
    galapagos::interface<word_t> * out
){
    jacobi_ctrl(id, in, out);
}

PGAS_METHOD(kern1, 0);
#endif
}
