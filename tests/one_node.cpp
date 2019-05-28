#include <iostream>

// #define USE_APUINT
// #include "am_globals.hpp"
#include "active_messages.hpp"
// typedef std::array<std::byte, 8> byte_array_t;

extern "C" void kern0(galapagos::stream <word_t> *in, galapagos::stream <word_t> *out);
extern "C" void kern1(galapagos::stream <word_t> *in, galapagos::stream <word_t> *out);

// extern void kern0(galapagos::stream <word_t> *in, galapagos::stream <word_t> *out);

int main(){
    short source = 0;
    short dest = 1;
    std::string my_address = "localhost";
    std::vector <std::string> kern_info;
    kern_info.push_back(my_address);
    kern_info.push_back(my_address);

    galapagos::node <word_t> node(kern_info, my_address);
    node.add_kernel(source, kern0);
    node.add_kernel(dest, kern1);

    kernel_done = (std::atomic_bool**) malloc(2*sizeof(std::atomic_bool*));

    node.start();
    node.end();
}
