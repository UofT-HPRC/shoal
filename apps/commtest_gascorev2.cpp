#include <cstddef>
#include <string.h>
#include <stdio.h>
#include "thegasnet_globals.hpp"
#include "thegasnet_core.hpp"
#include "platforms.hpp"

// #define hc_S1_request_send 128
// void S1_request_send(gasnet_token_t token, gasnet_handlerarg_t arg0);

// #define hc_S2_return_delay 129
// void S2_return_delay(gasnet_token_t token, gasnet_handlerarg_t arg0, gasnet_handlerarg_t arg1);

#define hc_S3_receive_delay 5
void S3_receive_delay(gasnet_token_t token, gasnet_handlerarg_t arg0, gasnet_handlerarg_t arg1, gasnet_handlerarg_t arg2);

#define hc_L0_receivestring 6
void L0_receivestring(gasnet_token_t token, void *buf, size_t nbytes);

#define hc_S0_barrier_increment 7
void S0_barrier_increment(gasnet_token_t token);


static gasnet_handlerentry_t handlers[] =
{
	// {
	// 	hc_S1_request_send,
	// 	(void(*)())S1_request_send
	// },
	// {
	// 	hc_S2_return_delay,
	// 	(void(*)())S2_return_delay
	// },
	{
		hc_S3_receive_delay,
		(void(*)())S3_receive_delay
	},
	{
		hc_L0_receivestring,
		(void(*)())L0_receivestring
	},
	{
		hc_S0_barrier_increment,
		(void(*)())S0_barrier_increment
	}
};

void barrier(); // no memory fence for remote messages!?!
	


START_NODE_GLOBALS{

	unsigned int received;
	char *rcvstring;

	unsigned int barrier_cnt;
	unsigned int mynode,nodes,prevnode,nextnode;
	
}END_NODE_GLOBALS;


int main(int argc, char **argv)
{
	NODE_GLOBALS_INIT(shared, received: 0, rcvstring: NULL, barrier_cnt: 0);
	
	#ifdef __MICROBLAZE__
		gasnet_seginfo_t segment_table_app[GASNET_NUM_NODES];
	#else // x86-64, ARM
		gasnet_seginfo_t *segment_table_app;
	#endif

	PRINT("Here we go.\r\n");

	char* mysharedmem = NULL;
	char* nextnodesharedmem;

	// pre-GASnet settings
	Timer_Init();
		
	// build handler table
	// --
	// Gasnet init, gasnet attach
	gasnet_init(&argc, &argv);
	shared->mynode = gasnet_mynode();
	shared->nodes = gasnet_nodes();

	shared->nextnode = (shared->mynode + 1) % shared->nodes;
	
	gasnet_attach(handlers, sizeof(handlers)/sizeof(gasnet_handlerentry_t), (uintptr_t)0x1000, (uintptr_t)0);

	#ifdef __x86_64__ARM__
		segment_table_app = (gasnet_seginfo_t*) malloc(gasnet_nodes() * sizeof(gasnet_seginfo_t));
	#endif
	gasnet_getSegmentInfo(segment_table_app,gasnet_nodes());
		COMPILER_BARRIER

	thegasnet_sharedMemoryReady(0);
	PRINT("Node %d ready.\r\n",gasnet_mynode());
	

	mysharedmem = (char*)(unsigned int long)segment_table_app[shared->mynode].addr;
	nextnodesharedmem = (char*)segment_table_app[shared->nextnode].addr;
	
	if (shared->mynode==0)
	{
		char *teststring = "Mr.Watson, come here, I want to see you.";
		
		strcpy((char*)mysharedmem,teststring);

        // printf("shared->nextnode: %d\n", shared->nextnode);
		// printf("mysharedmem: %p\n", mysharedmem);
		// printf("nextnodesharedmem: %p\n", nextnodesharedmem);

		gasnet_AMRequestLong0(shared->nextnode, shared->nextnode,hc_L0_receivestring,mysharedmem,strlen(teststring)+1,(nextnodesharedmem+256));
		Timer_Start();
	}
		
		
		COMPILER_BARRIER
	while(shared->received==0) sched_yield();
		COMPILER_BARRIER
	Timer_TakeTime();

	if (shared->mynode!=0)
		gasnet_AMRequestLong0(shared->nextnode, shared->nextnode,hc_L0_receivestring,(char*)shared->rcvstring,strlen(shared->rcvstring)+1,(nextnodesharedmem+256));
	#ifdef __x86_64__
		PRINT("Node %d received at address 0x%llX:\r\n",shared->mynode,(unsigned long long)shared->rcvstring); PRINT("    %s\r\n",shared->rcvstring);
	#else // Microblaze, ARM
		PRINT("Node %d received at address 0x%X:\r\n",shared->mynode,(unsigned int)shared->rcvstring); PRINT("    %s\r\n",shared->rcvstring);
	#endif

		
	if (shared->mynode==0)
	{
		PRINT("Message time: %d.%06ds\r\n",Timer_GetSecs(),Timer_GetuSecs());
	}

	/// TEMPORARY EXIT SOLUTION
	
	barrier();
	PRINT("Node %d done.\r\n",gasnet_mynode()); 
	
	// exit cleanly
	gasnet_exit(42);
	
	return 0;
}

// void S1_request_send(gasnet_token_t token, gasnet_handlerarg_t arg0) // arg0 = target
// {
// 	USE_NODE_GLOBALS(shared);
	
// 	gasnet_AMRequestShort2(arg0, hc_S2_return_delay, shared->mynode, 0);
// 	return;
// }

// void S2_return_delay(gasnet_token_t token, gasnet_handlerarg_t arg0, gasnet_handlerarg_t arg1) // arg0 = sender, arg1 = Sendtime
// {
// 	USE_NODE_GLOBALS(shared);
	
// 	gasnet_AMRequestShort3(0, hc_S3_receive_delay, arg0, shared->mynode, 0 - arg1);
// 	return;
// }

void S3_receive_delay(gasnet_token_t token, gasnet_handlerarg_t arg0, gasnet_handlerarg_t arg1, gasnet_handlerarg_t arg2)
{																			//arg0 = sender, arg1 = receiver, arg2 = Travel time
	//USE_NODE_GLOBALS(shared);
	//shared->traveltime[arg0][arg1] = arg2 * 10;
	//shared->rcvcnt++;
	return;
}

void L0_receivestring(gasnet_token_t token, void *buf, size_t nbytes)
{
	USE_NODE_GLOBALS(shared);
	
    printf("updating received\n");
	shared->rcvstring = (char*)buf;
	COMPILER_BARRIER
	shared->received++;

	return;
}

void S0_barrier_increment(gasnet_token_t token)
{
	USE_NODE_GLOBALS(shared);
	shared->barrier_cnt++;
	return;
}

void barrier()
{
	USE_NODE_GLOBALS(shared);
	
	unsigned int t;
	
	if (shared->mynode==0)
	{
		while(shared->barrier_cnt!=(shared->nodes)-1) 1;
		shared->barrier_cnt=0;
		COMPILER_BARRIER
		for(t=1;t<shared->nodes;t++)
			gasnet_AMRequestShort0(t, t, hc_S0_barrier_increment);
	}
	else
	{
		// Send message to 0
		gasnet_AMRequestShort0(0, 0, hc_S0_barrier_increment);
		COMPILER_BARRIER
		while(shared->barrier_cnt==0) 1;
		shared->barrier_cnt=0;
	}
	return;
}
