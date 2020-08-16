#include "jacobi.hpp"
#include "user_config.hpp"

DECLARE_METHOD(kern0);
DECLARE_METHOD(kern1);
DECLARE_METHOD(kern2);
// DECLARE_METHOD(kern3);
// DECLARE_METHOD(kern4);
// #if(KERN_BUILD == 1)
// DECLARE_METHOD(kern1);
// #endif
// #if(KERN_BUILD == -1 || KERN_BUILD == 2)
// DECLARE_METHOD(kern2);
// #endif

int main(){
    std::string address_0 = STRINGIFY(SHOAL_SW_0_IP_ADDR);
    std::string address_1 = STRINGIFY(SHOAL_SW_1_IP_ADDR);
    std::string address_2 = STRINGIFY(SHOAL_SW_2_IP_ADDR);
    std::string address_3 = STRINGIFY(10.1.2.6);
    std::string address_4 = STRINGIFY(10.1.2.7);
    std::vector <std::string> kern_info;
    for(int i = 0; i < (KERNEL_NUM_TOTAL-1) / NODES; i++){
        kern_info.push_back(address_1);
    }
    for(int i = 0; i < (KERNEL_NUM_TOTAL-1) / NODES; i++){
        kern_info.push_back(address_2);
    }
    for(int i = 0; i < (KERNEL_NUM_TOTAL-1) / NODES; i++){
        kern_info.push_back(address_3);
    }
    for(int i = 0; i < (KERNEL_NUM_TOTAL-1) / NODES; i++){
        kern_info.push_back(address_4);
    }
    kern_info.push_back(address_0);
    #if LOG_LEVEL > 0
    std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt("basic_logger", "main.log");
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    logger->flush_on(spdlog::level::debug);

    shoal::node node(kern_info, address_0, logger, true);
    #else
    shoal::node node(kern_info, address_0, true);
    #endif
    
    // for(int i = 0; i < KERNEL_NUM_TOTAL-1; i++){
    //     // node.add_kernel(i, kern2);
    //     node.add_kernel(i, kern0);
    // }
    node.add_kernel(KERNEL_NUM_TOTAL-1, kern1);

    // #if(KERN_BUILD == -1 || KERN_BUILD == 1)
    // node.add_kernel(KERN1_ID, kern1);
    // #endif
    // #if(KERN_BUILD == -1 || KERN_BUILD == 2)
    // node.add_kernel(KERN2_ID, kern2);
    // #endif

    // TIMESTAMP(node_0)
    // #if(KERN_BUILD == 0)
    node.init(KERNEL_NUM_TOTAL);
    // #elif(KERN_BUILD == 1 || KERN_BUILD == 2)
    // node.init(KERNEL_NUM_TOTAL);
    // #else
    // node.init(KERNEL_NUM_TOTAL);
    // #endif

    node.start();
    node.end();
}
