#include <iostream>

#include "node_2_kern_2.hpp"
#include "user_config.hpp"
// typedef std::array<std::byte, 8> byte_array_t;

// DECLARE_METHOD(kern0);
DECLARE_METHOD(kern1);

int main(){
    std::string address_0 = STRINGIFY(SHOAL_SW_0_IP_ADDR);
    std::string address_1 = STRINGIFY(SHOAL_SW_1_IP_ADDR);
    std::vector <std::string> kern_info;
    kern_info.push_back(address_0);
    kern_info.push_back(address_1);

    shoal::node node(kern_info, address_1);
    // node.add_kernel(KERN0_ID, kern0);
    node.add_kernel(KERN1_ID, kern1);

    node.init(KERNEL_NUM_1);

    node.start();
    node.end();
}
