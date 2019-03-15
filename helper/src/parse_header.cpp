#include <iostream>
#include <sstream>

#include "config.hpp"

int main(int argc, char* argv[]){

    std::istringstream ss(argv[1]);
    long header;
    if (!(ss >> header)) {
        std::cerr << "Invalid number: " << argv[1] << '\n';
    } else if (!ss.eof()) {
        std::cerr << "Trailing characters after number: " << argv[1] << '\n';
    }

    int src = (header >> AM_SRC_LOWER) & (AM_SRC_UPPER-AM_SRC_LOWER);
    int dst = (header >> AM_DST_LOWER) & (AM_DST_UPPER-AM_DST_LOWER);
    int payload_size = (header >> AM_PAYLOAD_SIZE_LOWER) & (AM_PAYLOAD_SIZE_UPPER-AM_PAYLOAD_SIZE_LOWER);
    int handler = (header >> AM_HANDLER_LOWER) & (AM_HANDLER_UPPER-AM_HANDLER_LOWER);
    int type = (header >> AM_TYPE_LOWER) & (AM_TYPE_UPPER-AM_TYPE_LOWER);
    int handler_args = (header >> AM_HANDLER_ARGS_LOWER) & (AM_HANDLER_ARGS_UPPER-AM_HANDLER_ARGS_LOWER);

    std::cout << "src: " << src << "\n";
    std::cout << "dst: " << dst << "\n";
    std::cout << "payload_size: " << payload_size << "\n";
    std::cout << "handler: " << handler << "\n";
    std::cout << std::hex << "type: " << type << "\n";
    std::cout << std::dec << "handler_args: " << handler_args << "\n";

    return 0;
    
}
