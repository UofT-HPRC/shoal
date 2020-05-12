#if !defined(SHOAL_INCLUDE_NODE_H_)
#define SHOAL_INCLUDE_NODE_H_

#include "active_messages.hpp"
#define CPU
#include "galapagos_node.hpp"
#include "galapagos_net_tcp.hpp"

namespace shoal{
    class node: public galapagos::node<word_t> {
        public:
            #if LOG_LEVEL > 0
            node(
                std::vector <std::string> & _kern_info_table,
                std::string  & _my_address,
                std::shared_ptr<spdlog::logger> logger
            ) : galapagos::node<word_t>(
                _kern_info_table, 
                _my_address,
                setup_network(_kern_info_table, _my_address, logger),
                logger
            ) {};
            #endif
            node(
                std::vector <std::string> & _kern_info_table,
                std::string  & _my_address
            ) : galapagos::node<word_t>(
                _kern_info_table, 
                _my_address,
                setup_network(_kern_info_table, _my_address)
            ) {};
            void init(int kernel_num);
            void end();
        private:
            #if LOG_LEVEL > 0
            static std::vector<galapagos::external_driver<word_t> *> setup_network(
                std::vector <std::string> & _kern_info_table,
                std::string &_my_address,
                std::shared_ptr<spdlog::logger> logger
            ){
                static galapagos::net::tcp <word_t> my_tcp(
                    7, // TCP port 
                    _kern_info_table, 
                    _my_address, 
                    logger
                );
                static std::vector < galapagos::external_driver<word_t> * > ext_drivers;
                ext_drivers.push_back(&my_tcp);
                return ext_drivers;
            }
            #endif
            static std::vector<galapagos::external_driver<word_t> *> setup_network(
                std::vector <std::string> & _kern_info_table,
                std::string &_my_address
            ){
                static galapagos::net::tcp <word_t> my_tcp(
                    7, // TCP port 
                    _kern_info_table, 
                    _my_address
                );
                static std::vector < galapagos::external_driver<word_t> * > ext_drivers;
                ext_drivers.push_back(&my_tcp);
                return ext_drivers;
            }
    };
} // namespace shoal

#endif // SHOAL_INCLUDE_NODE_H_
