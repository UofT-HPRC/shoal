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
            void wait_counter(unsigned int value);
            // TODO need separate streams for data / replies to prevent HoL blocking

            void barrier_send(int id);
            void barrier_wait();

            void sendShortAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                const word_t * handler_args);
            void sendShortAM_async(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                const word_t * handler_args);
            void sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                const word_t * handler_args, gc_payloadSize_t payloadSize);
            void sendPayload(gc_AMdst_t dst, word_t payload, bool assertLast);
            void sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                const word_t * handler_args, gc_payloadSize_t payloadSize, word_t src_addr);
            void sendMediumAM_async(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                const word_t * handler_args, gc_payloadSize_t payloadSize);
            void sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                const word_t * handler_args, gc_payloadSize_t payloadSize,
                word_t dst_addr);
            void sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount,
                const word_t * handler_args, gc_payloadSize_t payloadSize,
                word_t src_addr, word_t dst_addr);
            void sendLongStrideAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
                gc_payloadSize_t payloadSize, gc_stride_t src_stride, gc_strideBlockSize_t src_blk_size,
                gc_strideBlockNum_t src_blk_num, word_t src_addr, gc_stride_t dst_stride,
                gc_strideBlockSize_t dst_blk_size, gc_strideBlockNum_t dst_blk_num, word_t dst_addr);
            void sendLongVectorAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, const word_t * handler_args,
                gc_payloadSize_t payloadSize, gc_srcVectorNum_t srcVectorCount, gc_dstVectorNum_t dstVectorCount,
                const gc_vectorSize_t * srcSize, const gc_vectorSize_t * dstSize, const word_t * src_addr,
                const word_t * dst_addr);
    };
} // namespace shoal

#endif // SHOAL_INCLUDE_KERNEL_H_
