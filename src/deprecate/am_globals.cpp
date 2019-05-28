#include "am_globals.hpp"

void **handlertable = nullptr; // provided by user application, handler functions to run
mutex_t mutex_nodeInit; // mutex to initialize the node (per thread)

thread_local gasnet_nodedata_t* nodedata = nullptr; // used to hold a thread-local copy of gasnet_nodedata_all
thread_local void** gasnet_shared_mem = nullptr;

std::atomic_bool** kernel_done = nullptr;
