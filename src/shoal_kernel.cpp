#if defined(__MICROBLAZE__)

#include "shoal_kernel_hls.cpp"

#elif defined(__HLS__)

#include "shoal_kernel_hls.cpp"

#else

#include "shoal_kernel_x86.cpp"

#endif
