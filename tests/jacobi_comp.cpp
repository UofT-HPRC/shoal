#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <math.h>

#include "jacobi.hpp"

#define TIMECOND (myid==0)

#undef  INTERMEDIATE_TIMES

#define CELLTYPE int64_t

#ifdef __HLS__
// this function, if inlined, or placed in code, behaves badly in Verilog. In
// simulation, it does do three writes but they appear as burst writes to the wrong
// addresses. Doing it this way seems to work properly. Tested in HLS 2018.1
void start_timer_comp(volatile int* axi_timer){
    axi_timer[0] = 0x0; // stop timer
    axi_timer[1] = 0; // set load register to 0
    axi_timer[0] = 0x20; // load timer with load register
    axi_timer[0] = 0x80; // start timer
}

word_t get_time_comp(volatile int* axi_timer){
    // #pragma HLS INLINE
    // axi_timer[0] = 0x0; // stop timer
    word_t time = *(axi_timer + 0x2); // read timer count
	return time;
}

word_t accum_time_comp(word_t old_time, volatile int* axi_timer){
	word_t time = get_time_comp(axi_timer);
	return old_time + time;
}

void send_time_comp(shoal::kernel* kernel, gc_AMToken_t token, word_t time, volatile int* axi_timer){
    word_t dumb = *(axi_timer);
    if(dumb != 0xFF){
    kernel->sendMediumAM_async(KERNEL_NUM_TOTAL-1, token, H_EMPTY, 0, NULL, 8);
    kernel->sendPayload(KERNEL_NUM_TOTAL-1, time, true);
    }
    // axi_timer[0] = 0x0; // stop timer
    // kernel->wait_reply(1, axi_timer);
}
#else
auto get_time_comp(){
    return std::chrono::high_resolution_clock::now();
}

auto get_time_diff_comp(time_point_t timer){
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - timer;
    return elapsed.count();
}

double accum_time_comp(time_point_t old_time, double sum_time){
	double delta = get_time_diff_comp(old_time);
	double new_sum = sum_time + delta;
	return new_sum;
}
#endif

unsigned int log2floor_comp(unsigned int val)
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

unsigned int powerof2_comp(unsigned int val)
{
	unsigned int i;
	unsigned int res = 1;
	
	for(i=0 ; i<val ; i++)
		res *= 2;
	
	return res;
}

unsigned int ispowerof2_comp(unsigned int val)
{
	if ( powerof2_comp(log2floor_comp(val)) == val)
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

void sending_barrier(shoal::kernel* kernel, int ctrlnode){
	#pragma HLS INLINE
	kernel->barrier_send(ctrlnode);
}

extern "C"{
void jacobi_comp(
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
	unsigned int log2nodes = log2floor_comp((KERNEL_NUM_TOTAL-1));
	unsigned int xnodes = (powerof2_comp(log2nodes/2 + log2nodes%2));
	unsigned int ynodes = (powerof2_comp(log2nodes/2));	
		
	///////////////////////////////////
	// MEMORY ALLOCATION
	///////////////////////////////////
	
	size_t bsize_with_aura = ((X/xnodes)+2) * ((Y/ynodes)+2) * sizeof(CELLTYPE);
	size_t bsize_pow2_aligned;

	if (ispowerof2_comp(bsize_with_aura))
		bsize_pow2_aligned = bsize_with_aura;
	else
		bsize_pow2_aligned = powerof2_comp(log2floor_comp(bsize_with_aura)+1);

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
	start_timer_comp(axi_timer);
	#else
	auto timer = get_time_comp();
	time_point_t timer_init = timer;

	memset(mysharedmem,0,mem_available);
	#endif
	
	node_jacobidata_str njdata_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
	uint64_t *stats_squarederror;
	uint64_t stats_squarederror_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
	uint64_t *stats_gradient;
	uint64_t stats_gradient_master[KERNEL_NUM_TOTAL-1] __attribute__((aligned(64)));
	
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
	remoteMemN  = ((1     ) + (ydiv+0)*yoffset) * sizeof(CELLTYPE);
	remoteMemE  = ((1     ) + (    +1)*yoffset) * sizeof(CELLTYPE);
	remoteMemS  = ((1     ) + (    +1)*yoffset) * sizeof(CELLTYPE);
	remoteMemW  = ((0+xdiv) + (    +1)*yoffset) * sizeof(CELLTYPE);

	localMemN  = (voidp64)(((1     ) + (    +0)*yoffset) * sizeof(CELLTYPE));
	localMemE  = (voidp64)(((1+xdiv) + (    +1)*yoffset) * sizeof(CELLTYPE));
	localMemS  = (voidp64)(((1     ) + (ydiv+1)*yoffset) * sizeof(CELLTYPE));
	localMemW  = (voidp64)(((0     ) + (    +1)*yoffset) * sizeof(CELLTYPE));


	node_jacobidata_str njdata_local;
	
	// kernel.barrier_send(ctrlnode);
	
	#ifdef __HLS__
	word_t time;
	#else
	auto memory_time = get_time_diff_comp(timer_init);
	#endif

	PRINT("Node %d ready.\r\n",myid);
	
	#ifdef __HLS__
	word_t dumb = axi_timer[0];
	if(dumb != 0xFF){
	#endif
	// kernel.barrier_send(ctrlnode);
	sending_barrier(&kernel, ctrlnode);
	#ifdef __HLS__
	}
	#endif
	
	// Timer_Start();
	#ifdef __HLS__
	start_timer_comp(axi_timer);
	#else
	timer = get_time_comp();
	#endif

	CELLTYPE *dstBlock,*srcBlock;
	CELLTYPE cellsum;
	int print_thr = 1;

	// PARAMETER DISTRIBUTION / RECEPTION
	size_t _size;
	short _dest, _id;
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
	if(TIMECOND){
		PRINT("shoal payload size is %ld\n", njdata_local.xsize*sizeof(CELLTYPE));
	}

	free(njdata_local_ptr);
	#endif
	
	
	///////////////////////////////////
	// MAIN LOOP
	///////////////////////////////////
	
	for(t=0;t<ITERATIONS;t++)
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
		}

		///////////////////////////////////////////
		// SPINLOCK FOR COMPLETE MESSAGE RECEIVE //
		///////////////////////////////////////////
		if (even){
			srcBlock = localA; dstBlock = localB;
		}
		else{ //odd
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
			commtime = accum_time_comp(commtime, axi_timer);
			start_timer_comp(axi_timer);
			#else
			commtime = accum_time_comp(timer, commtime);
			timer = get_time_comp();
			#endif
		}
		#endif
		// barrier(ctrlnode);///
		// kernel.barrier_send(ctrlnode);
		sending_barrier(&kernel, ctrlnode);
		#ifdef INTERMEDIATE_TIMES
		PRINTACCTIME(TIMECOND,"Barrier time:",bar1secs,bar1usecs);
		#else
		if(TIMECOND){
			#ifdef __HLS__
			bar1time = accum_time_comp(bar1time, axi_timer);
			start_timer_comp(axi_timer);
			#else
			// ACCTIME(TIMECOND,bar1secs,bar1usecs);
			bar1time = accum_time_comp(timer, bar1time);
			timer = get_time_comp();
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
				
				*dstBlock = cellsum;

				diff = *srcBlock-*dstBlock; // used for average later
				
				//diffsqare = ;
				diffsquaresum += (int64_t)diff*(int64_t)diff;
				
				srcBlock++;
				dstBlock++;
			}
			// jump over aura cells
			srcBlock += 2;
			dstBlock += 2;
		}

		//////////////////////////////
		// STATS RECEPTION / BARRIER
		//////////////////////////////

		// COMPUTATION TIMING
		#ifdef INTERMEDIATE_TIMES
				PRINTACCTIME(TIMECOND,"Computation time:",compsecs,compusecs);
		#else
			if(TIMECOND){
				#ifdef __HLS__
				comptime = accum_time_comp(comptime, axi_timer);
				start_timer_comp(axi_timer);
				#else
				// ACCTIME(TIMECOND,compsecs,compusecs)
				comptime = accum_time_comp(timer, comptime);
				timer = get_time_comp();
				#endif
			}
				
		#endif
	
		const word_t increment = 1;
		#ifdef __HLS__
		kernel.sendMediumAM_async(ctrlnode, myid, H_ADD, 1, 16);
		kernel.sendPayload(ctrlnode, increment, false);
		#else
		kernel.sendMediumAM_async(ctrlnode, myid, H_ADD, 1, &increment, 16);
		#endif
		kernel.sendPayload(ctrlnode, diffsquaresum, false);
		kernel.sendPayload(ctrlnode, maxgrad, true);

		// kernel.barrier_send(ctrlnode);
		sending_barrier(&kernel, ctrlnode);
		COMPILER_BARRIER
	
		
		#ifdef INTERMEDIATE_TIMES
				PRINTACCTIME(TIMECOND,"Barrier time:",bar2secs,bar2usecs);
		#else
			if(TIMECOND){
				#ifdef __HLS__
				bar2time = accum_time_comp(bar2time, axi_timer);
				start_timer_comp(axi_timer);
				#else
				// ACCTIME(TIMECOND,bar2secs,bar2usecs)
				bar2time = accum_time_comp(timer, bar2time);
				timer = get_time_comp();
				#endif
			}
		#endif
	
		// if (shared->continue_running==-1) break; // leave iteration loop
			

	} // ITERATION LOOP
	
	
	if (TIMECOND){
		#ifdef __HLS__
		kernel.sendMediumAM_async(KERNEL_NUM_TOTAL-1, 2, H_EMPTY, 0, NULL, 32);
    	kernel.sendPayload(KERNEL_NUM_TOTAL-1, commtime, false);
		kernel.sendPayload(KERNEL_NUM_TOTAL-1, bar1time, false);
		kernel.sendPayload(KERNEL_NUM_TOTAL-1, comptime, false);
		kernel.sendPayload(KERNEL_NUM_TOTAL-1, bar2time, true);
		// // send_time_comp(&kernel, 0x2, commtime, axi_timer);
		// // send_time_comp(&kernel, 0x3, bar1time, axi_timer);
		// // send_time_comp(&kernel, 0x4, comptime, axi_timer);
		// // send_time_comp(&kernel, 0x5, bar2time, axi_timer);
		#else
		word_t tmp;
		kernel.sendMediumAM_async(KERNEL_NUM_TOTAL-1, 2, H_EMPTY, 0, NULL, 32);
		// memcpy(&tmp, &commtime, sizeof(double));
    	kernel.sendPayload(KERNEL_NUM_TOTAL-1, (commtime * 1E9 / 6.4), false);
		// memcpy(&tmp, &bar1time, sizeof(double));
    	kernel.sendPayload(KERNEL_NUM_TOTAL-1, (bar1time * 1E9 / 6.4), false);
		// memcpy(&tmp, &comptime, sizeof(double));
    	kernel.sendPayload(KERNEL_NUM_TOTAL-1, (comptime * 1E9 / 6.4), false);
		// memcpy(&tmp, &bar2time, sizeof(double));
    	kernel.sendPayload(KERNEL_NUM_TOTAL-1, (bar2time * 1E9 / 6.4), true);
		// kernel.sendPayload(KERNEL_NUM_TOTAL-1, bar1time, false);
		// kernel.sendPayload(KERNEL_NUM_TOTAL-1, comptime, false);
		// kernel.sendPayload(KERNEL_NUM_TOTAL-1, bar2time, true);
		// std::cout << "Cumulative communication time: " << commtime << "\n";
		// std::cout << "Cumulative barrier_1 time: " << bar1time << "\n";
		// std::cout << "Cumulative computation time: " << comptime << "\n";
		// std::cout << "Cumulative barrier_2 time: " << bar2time << "\n";
		#endif
	}

	// kernel.barrier_send(ctrlnode);
	sending_barrier(&kernel, ctrlnode);
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

void kern2(
    short id,
    galapagos::interface <word_t> * in,
    galapagos::interface<word_t> * out
){
    jacobi_comp(id, in, out);
}

PGAS_METHOD(kern2, 0);
#endif
}
