#include <iostream>
#include <sstream>

#include "config.hpp"
#include "global_utilities.hpp"

int main(int argc, char* argv[]){

    std::istringstream ss(argv[2]);
    long long header;

    if(*(argv[1]) == 'h'){
        if (!(ss >> std::hex >> header)) {
            std::cerr << "Invalid hex number: " << argv[2] << '\n';
            return -1;
        }
    } else if(*(argv[1]) == 'd'){
        if (!(ss >> header)) {
            std::cerr << "Invalid dec number: " << argv[2] << '\n';
            return -1;
        }
    } else{
        std::cerr << "Unknown base: " << argv[1] << '\n';
        return -1;
    }

    // if (!ss.eof()) {
    //     std::cerr << "Trailing characters after number: " << argv[2] << '\n';
    //     return -1;
    // }

    // int src = (header >> AM_SRC_LOWER) & (AM_SRC_UPPER-AM_SRC_LOWER);
    int src = hdextract(header, AM_SRC);
    // int dst = (header >> AM_DST_LOWER) & (AM_DST_UPPER-AM_DST_LOWER);
    int dst = hdextract(header, AM_DST);
    // int payload_size = (header >> AM_PAYLOAD_SIZE_LOWER) & (AM_PAYLOAD_SIZE_UPPER-AM_PAYLOAD_SIZE_LOWER);
    int payload_size = hdextract(header, AM_PAYLOAD_SIZE);
    // int handler = (header >> AM_HANDLER_LOWER) & (AM_HANDLER_UPPER-AM_HANDLER_LOWER);
    int handler = hdextract(header, AM_HANDLER);
    // int type = (header >> AM_TYPE_LOWER) & (AM_TYPE_UPPER-AM_TYPE_LOWER);
    int type = hdextract(header, AM_TYPE);
    // int handler_args = (header >> AM_HANDLER_ARGS_LOWER) & (AM_HANDLER_ARGS_UPPER-AM_HANDLER_ARGS_LOWER);
    int handler_args = hdextract(header, AM_HANDLER_ARGS);

    Color::Modifier red(Color::FG_RED);
    Color::Modifier def(Color::FG_DEFAULT);

    std::cout << "src: " << red << src << def << std::endl;
    std::cout << "dst: " << red << dst << def << std::endl;
    std::cout << "payload_size: " << red << payload_size << def << std::endl;
    std::cout << "handler: " << red << handler << def << std::endl;
    std::cout << "type: " << red << std::hex << type << def << std::endl;
    std::cout << "handler_args: " << red << handler_args << def << std::endl;

    return 0;
    
}
