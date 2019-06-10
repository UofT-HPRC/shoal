#if !defined(SHOAL_INCLUDE_AM_GLOBALS_H_)
#define SHOAL_INCLUDE_AM_GLOBALS_H_

#include <mutex>
#include <thread>
#include <atomic>
#include <cstddef>

typedef std::mutex mutex_t;
typedef std::lock_guard<mutex_t> lock_guard_t;
typedef std::unique_lock<mutex_t> lock_t;
typedef std::thread thread_t;

// defines information about the node
typedef struct gasnet_nodedata_str {
    unsigned int barrier_cnt;
    unsigned int mem_ready_barrier_cnt;
    unsigned int counter; // generic counter for H_ADD handler
    // TODO Add H_WAIT
} gasnet_nodedata_t;

#define INIT_SAFE_COUT \
	lock_t lock_print(mutex_print, std::defer_lock);

#define SAFE_COUT(args) \
lock_print.lock(); \
std::cout << args; \
lock_print.unlock();

#define ATOMIC_ACTION(args) \
lock_print.lock(); \
args; \
lock_print.unlock();

extern void **handlertable; // provided by user application, handler functions to run

// globally shared
extern mutex_t mutex_nodeInit; // mutex to initialize the node (per thread)
extern mutex_t mutex_print; // mutex to safely print
extern std::byte** gasnet_shared_mem_global;
extern gasnet_nodedata_t* gasnet_nodedata_all;

extern thread_local gasnet_nodedata_t* nodedata; // thread-copy of gasnet_nodedata_all
extern thread_local std::byte* gasnet_shared_mem;
extern thread_local lock_t lock_print;
extern thread_local mutex_t* mutex_nodedata;

extern std::atomic_bool** kernel_done;
extern mutex_t** mutex_nodedata_global;

#endif // SHOAL_INCLUDE_AM_GLOBALS_H_
