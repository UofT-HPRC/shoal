// platforms.h
// Toronto Heterogeneous GASNet
// Platform-specific definitions
// (c)2011-2014 Ruediger Willenberg

// Varun Sharma

#ifndef PLATFORMS_H
#define PLATFORMS_H

#define PRINTTIME(COND,TEXT){                       \
        Timer_TakeTime();                           \
        if((COND)){                                 \
            char TIMEOUTPUT[100];                   \
            sprintf(TIMEOUTPUT,TEXT " %d.%06ds\r\n",Timer_GetSecs(),Timer_GetuSecs()); \
            PRINT("%s",TIMEOUTPUT);                 \
        }                                           \
        Timer_Start();                              \
    }

#define PRINTACC(COND,SECVAR,USECVAR)				\
    {												\
        Timer_TakeTime();							\
        if((COND))									\
        { 											\
            unsigned int t_gs  = Timer_GetSecs();	\
            unsigned int t_gus = Timer_GetuSecs();	\
            USECVAR += t_gus;						\
            SECVAR  += t_gs + (USECVAR/1000000);	\
            USECVAR %= 1000000;						\
        }											\
        Timer_Start(); 								\
    }

#define PRINTACCTIME(COND,TEXT,SECVAR,USECVAR)		\
    {												\
        Timer_TakeTime();							\
        if((COND))									\
        { 											\
            unsigned int t_gs  = Timer_GetSecs();	\
            unsigned int t_gus = Timer_GetuSecs();	\
            USECVAR += t_gus;						\
            SECVAR  += t_gs + (USECVAR/1000000);	\
            USECVAR %= 1000000;						\
            char TIMEOUTPUT[100];					\
            sprintf(TIMEOUTPUT,TEXT " %d.%06ds\r\n",t_gs,t_gus);	\
            PRINT("%s",TIMEOUTPUT);					\
        }											\
        Timer_Start(); 								\
    }


#if defined (__MICROBLAZE__) || defined (__ARM__)
	#define __MICROBLAZE__ARM__
#endif

#if defined (__x86_64__) || defined (__ARM__)
	#define __x86_64__ARM__
#endif

#ifdef __MICROBLAZE__ // bare-metal, no OS

	#define COMPILER_BARRIER asm volatile ("" : : : "memory");
	
	#define READ_SHARED(X) (X)
	
	// includes
	#include "xparameters.h"
	#include "fsl.h"
	#ifdef TIMER_FSL
		#include "fsl_timer.h"
	#else
		#include "xtmrctr_l.h"     /* layer 0 timer counter device driver          */
		#include "xtmrctr.h"       /* layer 1 timer counter device driver          */
	#endif
		
	// timer
	void Timer_Init();
	void Timer_Start();
	void Timer_TakeTime();
	void Timer_TakeTimeRestart();
	unsigned int Timer_GetSecs();
	unsigned int Timer_GetuSecs();
	
	// printf
	#include "printfast.h"
	#if defined(STDOUT_BASEADDRESS) || defined(STDOUT_FSL)  // GASNET_NODE_ID==0
		#define PRINT(...) printfast(__VA_ARGS__)
	#else
		#define PRINT(...)
	#endif

	#if defined(STDIN_BASEADDRESS) || defined(STDIN_FSL)
		#define WAITKEYPRESS()  getfast()
	#else
		#define WAITKEYPRESS()
	#endif
	
	// type conversions
	typedef unsigned long long voidp64;
	
	#define ptr_uintL(x) ((unsigned int)(x))
	#define ptr_uintH(x) ((unsigned int)(0))
	#define ptr64_uintL(x) ((unsigned int)((voidp64)(x) & 0xFFFFFFFF))
	#define ptr64_uintH(x) ((unsigned int)(((voidp64)(x))>>32))
	#define uintHL_ptr(H,L)   ((void*)(L))
	#define uintHL_ptr64(H,L) ((((voidp64)(H))<<32 | ((voidp64)(L)&0xFFFFFFFF)))
	
	#define sched_yield()

#endif //__MICROBLAZE__

#ifdef __x86_64__ // Linux OS implied

	#define COMPILER_BARRIER __sync_synchronize();
	#define READ_SHARED(X) (__sync_add_and_fetch(&(X),0))
	
	// includes
	#include <time.h>
	#include <mutex>
	#include <condition_variable>
	#include <queue>
	#include <thread>

	// timer
	void Timer_Init();
	void Timer_Start();
	void Timer_TakeTime();
	void Timer_TakeTimeRestart();
	unsigned int Timer_GetSecs();
	unsigned int Timer_GetuSecs();
	
	// printf
	#define PRINT(...) printf(__VA_ARGS__);fflush(stdout)
	#define WAITKEYPRESS(...)  getchar(__VA_ARGS__)
	
	// type conversions
	typedef void* voidp64_t; //? could we make this void* instead of u.l.l., with appropriate conversion macros?
	typedef std::mutex mutex_t;
	typedef std::lock_guard<mutex_t> lock_guard_t;
	typedef std::unique_lock<mutex_t> lock_t;
	typedef std::condition_variable condition_t;
	typedef std::thread thread_t;
	
	#define ptr_uintL(x) ((unsigned int)((unsigned long)(x) & 0xFFFFFFFF))
	#define ptr_uintH(x) ((unsigned int)(((unsigned long)(x))>>32))
	#define ptr64_uintL(x) ptr_uintL(x)
	#define ptr64_uintH(x) ptr_uintH(x)
	#define uintHL_ptr(H,L) ((void*)(((unsigned long)(H))<<32 | ((unsigned long)(L)&0xFFFFFFFF)))
	#define uintHL_ptr64(H,L) uintHL_ptr(H,L)

	#define ON_ERROR(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }

#endif //__x86_64__

#ifdef __ARM__ // Linux OS, 32-bit ARM implied

	#define COMPILER_BARRIER  __sync_synchronize();
	
	#define READ_SHARED(X) (__sync_add_and_fetch(&(X),0))

	unsigned int print_cpu_affinity(char* name, unsigned int id);
	
	// timer
	void Timer_Init();
	void Timer_Start();
	void Timer_TakeTime();
	void Timer_TakeTimeRestart();
	unsigned int Timer_GetSecs();
	unsigned int Timer_GetuSecs();
	
	// printf
	#define PRINT(...) printf(__VA_ARGS__)
	#define WAITKEYPRESS(...)  getchar(__VA_ARGS__)
	
	// type conversions
	typedef unsigned long long voidp64;
	
	#define ptr_uintL(x) ((unsigned int)(x))
	#define ptr_uintH(x) ((unsigned int)(0))
	#define ptr64_uintL(x) ((unsigned int)((voidp64)(x) & 0xFFFFFFFF))
	#define ptr64_uintH(x) ((unsigned int)(((voidp64)(x))>>32))
	#define uintHL_ptr(H,L)   ((unsigned long*)(L))
	#define uintHL_ptr64(H,L) ((((voidp64)(H))<<32 | ((voidp64)(L)&0xFFFFFFFF)))

#endif //__ARM__

#endif // PLATFORMS_H
