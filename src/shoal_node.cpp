#include "shoal_node.hpp"
#include "am_globals.hpp"

void shoal::node::init(int kernel_num){

    gasnet_shared_mem_global = (std::byte**) calloc(kernel_num,sizeof(std::byte));
    kernel_done = (std::atomic_bool**) malloc(kernel_num*sizeof(std::atomic_bool*));
    mutex_nodedata_global = (mutex_t**) malloc(kernel_num*sizeof(mutex_t*));
    gasnet_nodedata_all = (gasnet_nodedata_t*) malloc(kernel_num*sizeof(gasnet_nodedata_t));
    std::cout << "Node initialized\n";

}

void shoal::node::end(){

    galapagos::node<word_t>::end();
    free(gasnet_shared_mem_global);
    free(kernel_done);
    free(gasnet_nodedata_all);
    std::cout << "Node finished\n";

}
