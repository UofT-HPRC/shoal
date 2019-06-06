#include "am_globals.hpp"

void **handlertable = nullptr; // provided by user application, handler functions to run
mutex_t mutex_nodeInit; // mutex to initialize the node (per thread)
mutex_t mutex_print; // mutex to safely print
std::byte* gasnet_shared_mem_global = nullptr;
gasnet_nodedata_t* gasnet_nodedata_all = nullptr;

thread_local gasnet_nodedata_t* nodedata = nullptr; // used to hold a thread-local copy of gasnet_nodedata_all
thread_local std::byte* gasnet_shared_mem = nullptr;
thread_local lock_t lock_print(mutex_print, std::defer_lock);

std::atomic_bool** kernel_done = nullptr;
