// Reudi Willenberb

#define _POSIX_C_SOURCE 199309L // needed for CLOCK_MONOTONIC
#include "platforms.hpp"
#include <stdint.h>

#ifdef __MICROBLAZE__ // bare-metal, no OS

	#ifdef TIMER_FSL
		static uint32_t time;
		static uint32_t timeL;
		static uint32_t timeH;

		static uint64_t time64;
		static uint64_t starttime64;
		static uint64_t stoptime64;
		static uint64_t difftime64;

		void Take_Time()
		{
			FSL_Timer_Snapshot(&timeL, &timeH);
			time64 = (((uint64_t)timeH)<<32) | (uint64_t)timeL;
		}
		
		void Timer_Init()
		{
			time = 0;
			timeL = 0;
			timeH = 0;
			time64 = 0;
			starttime64 = 0;
			stoptime64 = 0;
			FSL_Timer_Reset();
		}
		
		void Timer_Start()
		{
			Take_Time();
			starttime64 = time64;
		}
		
		void Timer_TakeTime()
		{
			Take_Time();
			stoptime64 = time64;
			difftime64 = stoptime64 - starttime64;
		}
		
		void Timer_TakeTimeRestart()
		{
			Take_Time();
			stoptime64 = time64;
			difftime64 = stoptime64 - starttime64;
			starttime64 = stoptime64;
		}
		
		unsigned int Timer_GetSecs()
		{
			return (uint32_t)(difftime64 / (uint64_t)100000000);
			
		}

		unsigned int Timer_GetuSecs()
		{
			return (uint32_t)((difftime64 % (uint64_t)100000000) / (uint64_t)100);

		}

	#else
		static XTmrCtr TimerCounter;//   __attribute__ ((unused));
		
		static uint32_t time;
		static uint32_t timeL;
		static uint32_t timeH;

		static uint64_t time64;
		static uint64_t starttime64;
		static uint64_t stoptime64;
		static uint64_t difftime64;
		
		void Take_Time()
		{
			time = XTmrCtr_GetValue(&TimerCounter, 1);
			timeL = XTmrCtr_GetValue(&TimerCounter, 0);
			timeH = XTmrCtr_GetValue(&TimerCounter, 1);
			if (time!=timeH)
				timeL = XTmrCtr_GetValue(&TimerCounter, 0);
			
			time64 = (((uint64_t)timeH)<<32) | (uint64_t)timeL;
		}
		
		void Timer_Init()
		{
			time = 0;
			timeL = 0;
			timeH = 0;
			time64 = 0;
			starttime64 = 0;
			stoptime64 = 0;
			XTmrCtr_Initialize(&TimerCounter, XPAR_TMRCTR_0_DEVICE_ID);
			XTmrCtr_SetOptions(&TimerCounter, 0, XTC_CASCADE_MODE_OPTION);
			XTmrCtr_Start(&TimerCounter, 0);
		}
		
		void Timer_Start()
		{
			Take_Time();
			starttime64 = time64;
		}
		
		void Timer_TakeTime()
		{
			Take_Time();
			stoptime64 = time64;
			difftime64 = stoptime64 - starttime64;
		}
		
		void Timer_TakeTimeRestart()
		{
			Take_Time();
			stoptime64 = time64;
			difftime64 = stoptime64 - starttime64;
			starttime64 = stoptime64;
		}
		
		unsigned int Timer_GetSecs()
		{
			return (uint32_t)(difftime64 / (uint64_t)100000000);
			
		}

		unsigned int Timer_GetuSecs()
		{
			return (uint32_t)((difftime64 % (uint64_t)100000000) / (uint64_t)100);

		}
	#endif

#endif //__MICROBLAZE__

#ifdef __x86_64__ARM__ // Linux OS implied

	static __thread struct timespec starttime;// __attribute__ ((unused));
	static __thread struct timespec stoptime;// __attribute__ ((unused));
	//static __thread struct timespec difftime;// __attribute__ ((unused));

	static __thread uint64_t starttime64;
	static __thread uint64_t stoptime64;
	static __thread uint64_t difftime64;

	void Timer_Init(){}
	
	void Timer_Start(){
		clock_gettime(CLOCK_MONOTONIC, &starttime);
		starttime64 = ((uint64_t)starttime.tv_sec)*1000000000 + ((uint64_t)starttime.tv_nsec);
	}
	
	void Timer_TakeTime(){
		clock_gettime(CLOCK_MONOTONIC, &stoptime);
		stoptime64  = ((uint64_t)stoptime.tv_sec)*1000000000  + ((uint64_t)stoptime.tv_nsec);
		difftime64 = stoptime64 - starttime64;
	}
	
	void Timer_TakeTimeRestart(){
		clock_gettime(CLOCK_MONOTONIC, &stoptime);
		stoptime64  = ((uint64_t)stoptime.tv_sec)*1000000000  + ((uint64_t)stoptime.tv_nsec);
		difftime64 = stoptime64 - starttime64;
		starttime64 = stoptime64;
	}
	
	unsigned int Timer_GetSecs(){
		return (unsigned int)(difftime64 / 1000000000);
	}

	unsigned int Timer_GetuSecs(){
		return (unsigned int)((difftime64 % 1000000000) / 1000);
	}

#endif //__x86_64__ARM__
