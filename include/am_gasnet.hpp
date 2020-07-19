#if !defined(SHOAL_INCLUDE_AM_GASNET_H_)
#define SHOAL_INCLUDE_AM_GASNET_H_

#include "hls_types.hpp"
#define CPU
#include "galapagos_interface.hpp"

typedef gc_AMargs_t garg;

#define PGAS_METHOD(name, index) \
    void __real_##name(short id, galapagos::interface <word_t> *in, galapagos::interface <word_t> *out);\
	void __wrap_##name (short id, galapagos::interface <word_t> *in, galapagos::interface <word_t> *out){\
		void (*fcnPtr)(short id, galapagos::interface <word_t> *, galapagos::interface <word_t> *) = __real_##name;\
        handler_thread(fcnPtr, id, in, out);\
	}

#define DECLARE_METHOD(name)\
	extern "C" void name(short id, galapagos::interface <word_t> *in, galapagos::interface <word_t> *out)

void handler_thread(void (*fcnPtr)(short id, galapagos::interface <word_t> * in,
    galapagos::interface <word_t> * out), short id, galapagos::interface <word_t> * in,
    galapagos::interface <word_t> * out);

#define PACKET_THRESHOLD 0

// in bytes. Used to buffer packets from the kernel. Packets cannot be larger for now
#define PACKET_BUFFER 16384

#endif // SHOAL_INCLUDE_AM_GASNET_H_
