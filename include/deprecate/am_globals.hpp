#if !defined(SHOAL_INCLUDE_AM_GLOBALS_H_)
#define SHOAL_INCLUDE_AM_GLOBALS_H_

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

#endif // SHOAL_INCLUDE_AM_GLOBALS_H_
