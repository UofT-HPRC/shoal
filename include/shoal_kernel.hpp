#if !defined(SHOAL_INCLUDE_KERNEL_H_)
#define SHOAL_INCLUDE_KERNEL_H_

//#define USE_APUINT
#include "hls_types.hpp"
#define CPU
#include "galapagos_stream.hpp"

typedef struct{
	gc_AMhandler_t index; // == 0 for don’t care
	void (*fnptr)();
} gasnet_handlerentry_t;

namespace shoal{
    class kernel{
        private:
            int id;
            int kernel_num;
            galapagos::stream <word_t> * in;
            galapagos::stream <word_t> * out;

            void allocate_handlerTable();
            void sendMemUpdate(gc_AMdst_t dst);
            void sendBarrierUpdate(gc_AMdst_t dst);
        public:
            kernel(int id, int kernel_num, galapagos::stream <word_t> * in, 
                galapagos::stream <word_t> * out);
            int get_id();
            int init();
            int attach(gasnet_handlerentry_t *table, int numentries, int size);
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
                word_t * handler_args, gc_payloadSize_t payloadSize, word_t* payload);
            void sendMediumAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, 
                word_t * handler_args, gc_payloadSize_t payloadSize, word_t src_addr);
            void sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, 
                word_t * handler_args, gc_payloadSize_t payloadSize, 
                word_t* payload, word_t dst_addr);
            void sendLongAM_normal(gc_AMdst_t dst, gc_AMToken_t token,
                gc_AMhandler_t handlerID, gc_AMargs_t handlerArgCount, 
                word_t * handler_args, gc_payloadSize_t payloadSize, 
                word_t src_addr, word_t dst_addr);
    };
} // namespace shoal

#endif // SHOAL_INCLUDE_KERNEL_H_
