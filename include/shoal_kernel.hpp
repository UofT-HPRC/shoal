#if !defined(SHOAL_INCLUDE_KERNEL_H_)
#define SHOAL_INCLUDE_KERNEL_H_

// USE_APUINT and CPU may need to be defined prior to these inclusions
#include "hls_types.hpp"
#ifndef __HLS__
#define CPU
#endif
#include "galapagos_packet.h"

#ifdef __HLS__

#define SAFE_COUT(args)
#define ATOMIC_ACTION(args) args

#else // x86 or MB
#include "galapagos_interface.hpp"
typedef struct{
	gc_AMhandler_t index; // == 0 for don’t care
	void (*fnptr)();
} gasnet_handlerentry_t;

#endif

namespace shoal{
    class kernel{
        private:
            int id;
            int kernel_num;
            galapagos::interface<word_t> * in;
            galapagos::interface<word_t> * out;

            #ifdef __HLS__
            volatile int* handler_ctrl;
            #else
            void allocate_handlerTable();
            #endif
            void sendMemUpdate(gc_AMdst_t dst);
            void sendBarrierUpdate(gc_AMdst_t dst);
        public:
            int get_id();
            int init();
            #ifdef __HLS__
            kernel(int id, int kernel_num, galapagos::interface<word_t> * in,
                galapagos::interface<word_t> * out, volatile int * handler_ctrl);
            #else
            kernel(int id, int kernel_num, galapagos::interface<word_t> * in,
                galapagos::interface<word_t> * out);
            int attach(gasnet_handlerentry_t *table, int numentries, int size);
            #endif
            void end();

            void wait_mem(unsigned int value);
            void wait_barrier(unsigned int value);
            void wait_reply(unsigned int value);
            // TODO need separate streams for data / replies to prevent HoL blocking

            void barrier_send(int id);
            void barrier_wait();

            void sendShortAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                word_t * handler_args);
            void sendShortAM_async(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                word_t * handler_args);
            void sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                word_t * handler_args, gc_payloadSize_t payloadSize);
            void sendPayload(gc_AMdst_t dst, word_t payload, bool assertLast);
            void sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                word_t * handler_args, gc_payloadSize_t payloadSize, word_t src_addr);
            void sendMediumAM_async(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                word_t * handler_args, gc_payloadSize_t payloadSize);
            void sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                word_t * handler_args, gc_payloadSize_t payloadSize,
                word_t dst_addr);
            void sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                word_t * handler_args, gc_payloadSize_t payloadSize,
                word_t src_addr, word_t dst_addr);
    };
} // namespace shoal

#endif // SHOAL_INCLUDE_KERNEL_H_
