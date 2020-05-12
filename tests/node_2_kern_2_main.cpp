#include <iostream>

#include "node_2_kern_2.hpp"
#include "user_config.hpp"
// typedef std::array<std::byte, 8> byte_array_t;

#if(KERN_BUILD == -1 || KERN_BUILD == 0)
DECLARE_METHOD(kern0);
#endif
#if(KERN_BUILD == -1 || KERN_BUILD == 1)
DECLARE_METHOD(kern1);
#endif

int main(){
    std::string address_0 = STRINGIFY(SHOAL_SW_0_IP_ADDR);
    std::string address_1 = STRINGIFY(SHOAL_SW_1_IP_ADDR);
    std::vector <std::string> kern_info;
    kern_info.push_back(address_0);
    kern_info.push_back(address_1);

    #if LOG_LEVEL > 0
    std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt("basic_logger", "main.log");
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    logger->flush_on(spdlog::level::debug);
    #endif

    #if LOG_LEVEL > 0
    #if(KERN_BUILD == -1 || KERN_BUILD == 0)
    shoal::node node(kern_info, address_0, logger);
    node.add_kernel(KERN0_ID, kern0);
    #endif
    #if(KERN_BUILD == -1 || KERN_BUILD == 1)
    shoal::node node(kern_info, address_1, logger);
    node.add_kernel(KERN1_ID, kern1);
    #endif
    #else
    #if(KERN_BUILD == -1 || KERN_BUILD == 0)
    shoal::node node(kern_info, address_0);
    node.add_kernel(KERN0_ID, kern0);
    #endif
    #if(KERN_BUILD == -1 || KERN_BUILD == 1)
    shoal::node node(kern_info, address_1);
    node.add_kernel(KERN1_ID, kern1);
    #endif
    #endif

    #if(KERN_BUILD == -1 || KERN_BUILD == 0)
    node.init(KERNEL_NUM_0);
    #endif
    #if(KERN_BUILD == -1 || KERN_BUILD == 1)
    node.init(KERNEL_NUM_1);
    #endif

    node.start();
    node.end();
}
