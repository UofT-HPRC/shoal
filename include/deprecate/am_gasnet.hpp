#if !defined(SHOAL_INCLUDE_AM_GASNET_H_)
#define SHOAL_INCLUDE_AM_GASNET_H_

#include "hls_types.hpp"
#define CPU
#include "galapagos_stream.hpp"

#define SHARED_VARIABLE_NUM 256

typedef struct{
	gc_AMhandler_t index; // == 0 for don’t care
	void (*fnptr)();
} gasnet_handlerentry_t;

typedef gc_AMargs_t garg;

#define PGAS_METHOD(name, id) \
    void __real_##name(galapagos::stream <word_t> *in, galapagos::stream <word_t> *out);\
	void __wrap_##name (galapagos::stream <word_t> *in, galapagos::stream <word_t> *out){\
		void (*fcnPtr)(galapagos::stream <word_t> *, galapagos::stream <word_t> *) = __real_##name;\
        handler_thread(fcnPtr, id, in, out);\
	}

int gasnet_attach(gasnet_handlerentry_t *table, int numentries);

void handler_thread(void (*fcnPtr)(galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out), int id, galapagos::stream <word_t> * in, 
    galapagos::stream <word_t> * out);

#endif // SHOAL_INCLUDE_AM_GASNET_H_
