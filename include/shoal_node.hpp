#if !defined(SHOAL_INCLUDE_NODE_H_)
#define SHOAL_INCLUDE_NODE_H_

#include "active_messages.hpp"
#define CPU
#include "galapagos_node.hpp"

namespace shoal{
    class node: public galapagos::node<word_t> {
        public:
            node(std::vector <std::string> & _kern_info_table,
                std::string  & _my_address, std::shared_ptr<spdlog::logger> logger) :
                galapagos::node<word_t>(_kern_info_table, _my_address,
                    std::vector<galapagos::external_driver <word_t> * >(),
                    logger){};
            void init(int kernel_num);
            void end();
    };
} // namespace shoal

#endif // SHOAL_INCLUDE_NODE_H_
