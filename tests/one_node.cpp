#include <iostream>

#include "one_node.hpp"
// typedef std::array<std::byte, 8> byte_array_t;

DECLARE_METHOD(kern0);
DECLARE_METHOD(kern1);

int main(){
    std::string my_address = "localhost";
    std::vector <std::string> kern_info;
    kern_info.push_back(my_address);
    kern_info.push_back(my_address);

    shoal::node node(kern_info, my_address);
    node.add_kernel(KERN0_ID, kern0);
    node.add_kernel(KERN1_ID, kern1);

    node.init(KERNEL_NUM);

    node.start();
    node.end();
}
