#include "stencil_ctrl.hpp"

#define CELLTYPE uint_64_t

// this function, if inlined, or placed from_GAScore code, behaves badly from_GAScore Verilog. from_GAScore
// simulation, it does do three writes but they appear as burst writes to the wrong
// addresses. Doing it this way seems to work properly. Tested from_GAScore HLS 2018.1
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

void send_time_comp(shoal::kernel* kernel, gc_AMToken_t token, word_t time, volatile int* axi_timer, int kernel_num_total){
    word_t dumb = *(axi_timer);
    if(dumb != 0xFF){
    kernel->sendMediumAM_async(kernel_num_total-1, token, H_EMPTY, 0, NULL, 8);
    kernel->sendPayload(kernel_num_total-1, time, true);
    }
    // axi_timer[0] = 0x0; // stop timer
    // kernel->wait_reply(1, axi_timer);
}

void sending_barrier(shoal::kernel* kernel, int ctrlnode){
	#pragma HLS INLINE
	kernel->barrier_send(ctrlnode);
}

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

void stencil_ctrl(
    short id,
    galapagos::interface <word_t> * from_GAScore,
	galapagos::interface<word_t> * to_GAScore,
    galapagos::interface<ap_uint<32> > * to_stencil,
    galapagos::interface<ap_uint<32> > * from_stencil,
    galapagos::interface<ap_uint<8> > * to_stencil_ctrl,
    galapagos::interface<ap_uint<8> > * from_stencil_ctrl,
    volatile int * handler_ctrl,
	volatile int * axi_timer,
	int kernel_num_total,
	int grid_size
){

	#pragma HLS INTERFACE axis port=from_GAScore
    #pragma HLS INTERFACE axis port=to_GAScore
	#pragma HLS INTERFACE axis port=to_stencil
    #pragma HLS INTERFACE axis port=from_stencil
	#pragma HLS INTERFACE axis port=to_stencil_ctrl
    #pragma HLS INTERFACE axis port=from_stencil_ctrl
    #pragma HLS INTERFACE ap_ctrl_none port=return
    #pragma HLS INTERFACE ap_stable port=id
	#pragma HLS INTERFACE ap_stable port=kernel_num_total
	#pragma HLS INTERFACE ap_stable port=grid_size
    #pragma HLS INTERFACE m_axi port=handler_ctrl depth=4096 offset=0
    #pragma HLS INTERFACE m_axi port=axi_timer depth=4096 offset=0

    // time measurement variables
    word_t commtime = 0;
	word_t comptime = 0;
	word_t bar1time = 0;
	word_t bar2time = 0;

    shoal::kernel kernel(id, kernel_num_total, from_GAScore, to_GAScore, handler_ctrl);
    kernel.init();

    // some constants ----------------------------------------------------------
    int myid = kernel.get_id();
    int ctrlnode  = kernel_num_total - 1; 	// e.g. Node #16
    const unsigned int log2nodes = log2floor_comp((kernel_num_total-1));
    const unsigned int xnodes = (powerof2_comp(log2nodes/2 + log2nodes%2));
    const unsigned int ynodes = (powerof2_comp(log2nodes/2));
    const unsigned int X = grid_size;
    const unsigned int Y = grid_size;
    const unsigned int ABoffset = 134217728; // 128 MiB since HW memory is hardcoded to 256MB

    sending_barrier(&kernel, ctrlnode); // wait until ctrl node has prepared the pole data

    // get the node data from ctrl node
    size_t _size;
	short _dest, _id;
    node_jacobidata_str njdata_local;
    galapagos::stream_packet <word_t> axis_word;
    ap_uint<32> njdata [10];
    ap_uint<4> njdata_index = 0;
    from_GAScore->read(); // read token
    for(int q = 0; q < sizeof(node_jacobidata_str)/GC_DATA_BYTES; q++){
        from_GAScore->read(axis_word);
        njdata[njdata_index++] = axis_word.data & 0xFFFFFFFF;
        njdata[njdata_index++] = ((axis_word.data) >> 32) & 0xFFFFFFFF;
    }

    // pass on to stencil core
    int i;
    galapagos::stream_packet <ap_uint<32> > stencil_word;
    stencil_word.user = 0;
    stencil_word.keep = 0xFFFF;
    stencil_word.id = 0;
    stencil_word.dest = 0;
    for(i = 0; i < 10; i++){
        stencil_word.data = njdata[i];
		stencil_word.last = i == 9;
        to_stencil->write(stencil_word);
    }

    ap_uint<32> xsize = njdata[0];
    ap_uint<32> ysize = njdata[1];
    unsigned int xdiv = X / xnodes;
	unsigned int ydiv = Y / ynodes;
	ptrdiff_t yoffset = xdiv + 2;
    //local node properties
	unsigned int myX = (myid) % xnodes;
	unsigned int myY = (myid) / xnodes;
	unsigned int myXmin = myX * xdiv;
	unsigned int myXmax = (myX+1)*xdiv - 1;
	unsigned int myYmin = myY * ydiv;
	unsigned int myYmax = (myY+1)*ydiv - 1;
	
	// determine neighbour node information (2D-Torus)
	unsigned int nodeN  = ((myY+ynodes-1)%ynodes) * xnodes + ((myX+xnodes+0)%xnodes);
	unsigned int nodeE  = ((myY+ynodes+0)%ynodes) * xnodes + ((myX+xnodes+1)%xnodes);
	unsigned int nodeS  = ((myY+ynodes+1)%ynodes) * xnodes + ((myX+xnodes+0)%xnodes);
	unsigned int nodeW  = ((myY+ynodes+0)%ynodes) * xnodes + ((myX+xnodes-1)%xnodes);
	
    // send to neighbours? -- special cases for edges
	bool sendN = (myY!=0);
	bool sendS = (myY!=(ynodes-1));
	bool sendW = (myX!=0);
	bool sendE = (myX!=(xnodes-1));

	// determine neighbour cell information
	long celloffsetN  = +0-yoffset;
	long celloffsetE  = +1;
	long celloffsetS  = -0+yoffset;
	long celloffsetW  = -1;

	// get neighbour memory localities
	voidp64 remoteMemN  = ((1     ) + (ydiv+0)*yoffset) * sizeof(CELLTYPE);
	voidp64 remoteMemE  = ((1     ) + (    +1)*yoffset) * sizeof(CELLTYPE);
	voidp64 remoteMemS  = ((1     ) + (    +1)*yoffset) * sizeof(CELLTYPE);
	voidp64 remoteMemW  = ((0+xdiv) + (    +1)*yoffset) * sizeof(CELLTYPE);

	voidp64 localMemN  = (voidp64)(((1     ) + (    +0)*yoffset) * sizeof(CELLTYPE));
	voidp64 localMemE  = (voidp64)(((1+xdiv) + (    +1)*yoffset) * sizeof(CELLTYPE));
	voidp64 localMemS  = (voidp64)(((1     ) + (ydiv+1)*yoffset) * sizeof(CELLTYPE));
	voidp64 localMemW  = (voidp64)(((0     ) + (    +1)*yoffset) * sizeof(CELLTYPE));


    // wait for stencil to read the node data (we should read 2)
	// seems like we're reading out 3 instead. Instead of reading here, just
	// read once after the get is complete.
    galapagos::stream_packet <ap_uint<8> > ctrl_word;
	ctrl_word.user = 0;
	ctrl_word.last = 0;
	ctrl_word.keep = 0xFFFF;
	ctrl_word.id = 0;
	ctrl_word.dest = 0;
	ctrl_word.data = 1;
	to_stencil_ctrl->write(ctrl_word);
    from_stencil_ctrl->read(ctrl_word);

    for(i = 0; i < ITERATIONS; i++){
        unsigned int odd = i % 2;
		unsigned int even = !odd;
		
		ptrdiff_t yoffset = xsize + 2;

        if(myid == 0){
            start_timer_comp(axi_timer);
        }

		if (sendN){
			kernel.getLongAM_normal(nodeN, 0x01, H_INCR_MEM, 0, nullptr, xsize*sizeof(CELLTYPE), (word_t)(remoteMemN +(odd*ABoffset)), (word_t)(localMemN +(odd*ABoffset)));
			kernel.wait_mem(1);
		}
		if (sendS){
			kernel.getLongAM_normal(nodeS, 0x02, H_INCR_MEM, 0, nullptr, xsize*sizeof(CELLTYPE), (word_t)(remoteMemS +(odd*ABoffset)), (word_t)(localMemS +(odd*ABoffset)));
			kernel.wait_mem(1);
		}
		if (sendW){
			kernel.getLongStrideAM_normal(nodeW, 0x03, H_INCR_MEM, 0, nullptr, ysize*sizeof(CELLTYPE), 
				yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), ysize, (word_t)(remoteMemW+(odd*ABoffset)),
				yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), ysize, (word_t)(localMemW+(odd*ABoffset))
			);
			kernel.wait_mem(1);
		}
		if (sendE){
			kernel.getLongStrideAM_normal(nodeE, 0x04, H_INCR_MEM, 0, nullptr, ysize*sizeof(CELLTYPE), 
				yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), ysize, (word_t)(remoteMemE+(odd*ABoffset)),
				yoffset*sizeof(CELLTYPE), sizeof(CELLTYPE), ysize, (word_t)(localMemE+(odd*ABoffset))
			);
			kernel.wait_mem(1);
		}
		
		if(myid == 0){
			commtime = accum_time_comp(commtime, axi_timer);
			start_timer_comp(axi_timer);
		}

		sending_barrier(&kernel, ctrlnode);

        if(myid == 0){
			bar1time = accum_time_comp(bar1time, axi_timer);
			start_timer_comp(axi_timer);
		}

        // tell core to start
        to_stencil_ctrl->write(ctrl_word);

        // read 1 from stencil when it's done
		to_stencil_ctrl->write(ctrl_word);
        from_stencil_ctrl->read(ctrl_word);

        // update computation time
		if(myid == 0){
			comptime = accum_time_comp(comptime, axi_timer);
			start_timer_comp(axi_timer);
		}

        // read stats. First the squared error, then maxgrad
		ap_uint<32> stats [3];
		if(myid < kernel_num_total){
        from_stencil->read(stencil_word);
        stats[0] = stencil_word.data;
        from_stencil->read(stencil_word);
        stats[1] = stencil_word.data;
        from_stencil->read(stencil_word);
        stats[2] = stencil_word.data;
		}

        // send stats to ctrl node
        const word_t increment = 1;
		ap_uint<64> sq_error = ((word_t)(stats[1]) << 32) | stats[0];
        kernel.sendMediumAM_async(ctrlnode, myid, H_ADD, 1, 16);
        kernel.sendPayload(ctrlnode, increment, false);
        kernel.sendPayload(ctrlnode, sq_error, false);
        kernel.sendPayload(ctrlnode, stats[2], true);

        // iteration done barrier
        sending_barrier(&kernel, ctrlnode);

        // update barrier 2 time
		if(myid == 0)
        	bar2time = accum_time_comp(bar2time, axi_timer);
    }

    // send timing data
    if (myid == 0){
		kernel.sendMediumAM_async(kernel_num_total-1, 2, H_EMPTY, 0, NULL, 32);
    	kernel.sendPayload(kernel_num_total-1, commtime, false);
		kernel.sendPayload(kernel_num_total-1, bar1time, false);
		kernel.sendPayload(kernel_num_total-1, comptime, false);
		kernel.sendPayload(kernel_num_total-1, bar2time, true);
	}

	// kernel.barrier_send(ctrlnode);
	sending_barrier(&kernel, ctrlnode);

	kernel.end();

	#ifdef __HLS__
	// dummy write to convey that we got to this point
	axi_timer[0xab] = 0xcd;

	while(1){};
	#endif

}