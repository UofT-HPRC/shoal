#include "hls_kernel.hpp"
#include "user_config.hpp"

DECLARE_METHOD(kern1);

int main(){
    std::string address_0 = STRINGIFY(SHOAL_SW_0_IP_ADDR);
    std::string address_1 = STRINGIFY(SHOAL_SW_1_IP_ADDR);
    std::vector <std::string> kern_info;
    kern_info.push_back(address_0);
    kern_info.push_back(address_1);
    std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt("basic_logger", "main.log");
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    logger->flush_on(spdlog::level::debug);

    shoal::node node(kern_info, address_0, logger);
    node.add_kernel(KERN1_ID, kern1);

    TIMESTAMP(node_0)
    node.init(KERNEL_NUM_1);

    TIMESTAMP(node_1)
    node.start();
    TIMESTAMP(node_2)
    node.end();

    TIMESTAMP_INIT
    TIMESTAMP_DIFF(node_1, node_0, "Node 1 Init")
    TIMESTAMP_DIFF(node_2, node_1, "Node 1 Start")
    TIMESTAMP_END
}
