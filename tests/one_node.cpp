#include <iostream>

#include "one_node.hpp"
// typedef std::array<std::byte, 8> byte_array_t;

extern "C" void kern0(galapagos::stream <word_t> *in, galapagos::stream <word_t> *out);
extern "C" void kern1(galapagos::stream <word_t> *in, galapagos::stream <word_t> *out);

int main(){
    std::string my_address = "localhost";
    std::vector <std::string> kern_info;
    kern_info.push_back(my_address);
    kern_info.push_back(my_address);

    galapagos::node <word_t> node(kern_info, my_address);
    node.add_kernel(KERN0_ID, kern0);
    node.add_kernel(KERN1_ID, kern1);

    // gasnet_shared_mem_global = (void*) new word_t[KERNEL_NUM*SHARED_VARIABLE_NUM];
    gasnet_shared_mem_global = (std::byte*) calloc(KERNEL_NUM*SHARED_VARIABLE_NUM,sizeof(std::byte));
    kernel_done = (std::atomic_bool**) malloc(KERNEL_NUM*sizeof(std::atomic_bool*));
    gasnet_nodedata_all = (gasnet_nodedata_t*) malloc(KERNEL_NUM*sizeof(gasnet_nodedata_t));
    // sync_cout = new Async::dlog();

    node.start();
    node.end();

    free(gasnet_shared_mem_global);
    free(kernel_done);
    // delete sync_cout;
}
