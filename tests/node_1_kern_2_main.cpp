#include <iostream>

#include "node_1_kern_2.hpp"
#include "user_config.hpp"
// typedef std::array<std::byte, 8> byte_array_t;

DECLARE_METHOD(kern0);
DECLARE_METHOD(kern1);

int main(){
    std::string my_address = STRINGIFY(SHOAL_SW_0_IP_ADDR);
    std::vector <std::string> kern_info;
    kern_info.push_back(my_address);
    kern_info.push_back(my_address);
    std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt("basic_logger", "main.log");

    shoal::node node(kern_info, my_address, logger);
    node.add_kernel(KERN0_ID, kern0);
    node.add_kernel(KERN1_ID, kern1);

    TIMESTAMP(node_0)
    node.init(KERNEL_NUM_0);

    TIMESTAMP(node_1)
    node.start();
    TIMESTAMP(node_2)
    node.end();

    TIMESTAMP_INIT
    TIMESTAMP_DIFF(node_1, node_0, "Node 0 Init")
    TIMESTAMP_DIFF(node_2, node_1, "Node 1 Start")
    TIMESTAMP_END
}
