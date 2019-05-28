#if !defined(SHOAL_INCLUDE_ACTIVE_MESSAGES_X86_)
#define SHOAL_INCLUDE_ACTIVE_MESSAGES_X86_

#include "config.hpp"
#include "hls_types.hpp"

#define CPU
#include "galapagos_node.hpp"

typedef struct{
	gc_AMhandler_t index; // == 0 for don’t care
	void (*fnptr)();
} gasnet_handlerentry_t;

typedef gc_AMargs_t garg;

#define PGAS_METHOD(name, id) \
    void __real_##name(galapagos::stream <word_t> *in, galapagos::stream <word_t> *out);\
	void __wrap_##name (galapagos::stream <word_t> *in, galapagos::stream <word_t> *out){\
		void (*fcnPtr)(galapagos::stream <word_t> *, galapagos::stream <word_t> *) = __real_##name;\
        handler_thread(fcnPtr, id, in, out);\
	}

////////////////////////////////////////

#include <mutex>
#include <thread>
#include <atomic>

typedef std::mutex mutex_t;
typedef std::lock_guard<mutex_t> lock_guard_t;
typedef std::unique_lock<mutex_t> lock_t;
typedef std::thread thread_t;

// defines information about the node
typedef struct gasnet_nodedata_str {
    int barrier_cnt;
    unsigned int mem_ready_barrier_cnt;
    unsigned int counter; // generic counter for H_ADD handler
    // TODO Add H_WAIT
} gasnet_nodedata_t;

extern void **handlertable; // provided by user application, handler functions to run

// globally shared
extern mutex_t mutex_nodeInit; // mutex to initialize the node (per thread)

extern thread_local gasnet_nodedata_t* nodedata; // thread-copy of gasnet_nodedata_all
extern thread_local void** gasnet_shared_mem;

extern std::atomic_bool** kernel_done;

/////////////////////////////////////

int gasnet_attach(gasnet_handlerentry_t *table, int numentries);

void handler_thread(void (*fcnPtr)(galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out), int id, galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out);

galapagos::stream_packet <word_t> createHeaderBeat(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_payloadSize_t payloadSize,
    gc_AMhandler_t handlerID,
    gc_AMtype_t type,
    gc_AMargs_t handlerArgCount
);

galapagos::stream_packet <word_t> createTokenBeat(
    gc_AMToken_t token,
    bool last
);

void sendHandlerArgs(
    galapagos::stream <word_t> & axis_out,
    word_t * handler_args,
    gc_AMargs_t handlerArgCount,
    bool assertLast
);

void sendPayloadArgs(
    galapagos::stream <word_t> & axis_out,
    word_t * payload_args,
    gc_payloadSize_t payloadArgCount,
    bool assertLast
);

void sendMediumAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    gc_payloadSize_t payloadSize,
    word_t * payload,
    galapagos::stream <word_t> & out
);

#endif // SHOAL_INCLUDE_ACTIVE_MESSAGES_X86_
