#include "shoal_node.hpp"
#include "am_globals.hpp"

void shoal::node::init(int kernel_num){

    gasnet_shared_mem_global = (std::byte**) calloc(kernel_num,sizeof(std::byte*));
    kernel_done = (std::atomic_bool**) malloc(kernel_num*sizeof(std::atomic_bool*));
    mutex_nodedata_global = (mutex_t**) malloc(kernel_num*sizeof(mutex_t*));
    gasnet_nodedata_all = (gasnet_nodedata_t*) calloc(kernel_num, sizeof(gasnet_nodedata_t));
    // std::cout << "Node initialized\n";

}

// void shoal::node::add_kernel(
//     short id, 
//     void (*func)(short , galapagos::interface <word_t> *, galapagos::interface <word_t> *)
// ){

//     void __real_kern0(short id, galapagos::interface <word_t> *in, galapagos::interface <word_t> *out);
//     void __wrap_kern0 (short id, galapagos::interface <word_t> *in, galapagos::interface <word_t> *out){
//         void (*fcnPtr)(short id, galapagos::interface <word_t> *, galapagos::interface <word_t> *) = __real_kern0;
//         handler_thread(fcnPtr, id, in, out);
//     }

//     thread_t handler_thread = std::thread(func, id, &kernel_in, &kernel_out);

//     galapagos::node<word_t>::end();
//     free(gasnet_shared_mem_global);
//     free(kernel_done);
//     free(mutex_nodedata_global);
//     free(gasnet_nodedata_all);
//     std::cout << "Node finished\n";

// }

void shoal::node::end(){

    galapagos::node<word_t>::end();
    free(gasnet_shared_mem_global);
    free(kernel_done);
    free(mutex_nodedata_global);
    free(gasnet_nodedata_all);
    std::cout << "Node finished\n";

}
