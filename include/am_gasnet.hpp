#if !defined(SHOAL_INCLUDE_AM_GASNET_H_)
#define SHOAL_INCLUDE_AM_GASNET_H_

#include "hls_types.hpp"
#define CPU
#include "galapagos_stream.hpp"

typedef gc_AMargs_t garg;

#define PGAS_METHOD(name, id) \
    void __real_##name(galapagos::stream <word_t> *in, galapagos::stream <word_t> *out);\
	void __wrap_##name (galapagos::stream <word_t> *in, galapagos::stream <word_t> *out){\
		void (*fcnPtr)(galapagos::stream <word_t> *, galapagos::stream <word_t> *) = __real_##name;\
        handler_thread(fcnPtr, id, in, out);\
	}

#define DECLARE_METHOD(name)\
	extern "C" void name(galapagos::stream <word_t> *in, galapagos::stream <word_t> *out)

void handler_thread(void (*fcnPtr)(galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out), int id, galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out);

#endif // SHOAL_INCLUDE_AM_GASNET_H_
