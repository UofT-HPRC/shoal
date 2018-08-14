#include <fstream>
#include <iostream>
#include <cstdlib>
#include "xpams.hpp"
#include "testbench.hpp"

#define DAT_FILE "/GASCore/testbench/xpams.dat" //relative to repo root

#ifdef DEBUG
#define CALL_TB xpams(dbg_currentState, axis_rx, axis_tx_handler,axis_kernel_out,axis_kernel_in, \
        axis_tx_kernel, blockingWait,AMcounter_threshold, \
        wordCounter_threshold, customCounter_master, handlerCounter_threshold, \
        handlerCounter_master, enable, mask);
#else
#define CALL_TB xpams(axis_rx, axis_tx_handler,axis_kernel_out,axis_kernel_in, \
        axis_tx_kernel, blockingWait,AMcounter_threshold, \
        wordCounter_threshold, customCounter_master, handlerCounter_threshold, \
        handlerCounter_master, enable, mask);
#endif

#define PRINT_AXIS std::cout << "Stream statuses:\n"; \
    std::cout << "  Rx: " << std::hex << axis_rx.size() << "\n"; \
    std::cout << "  Tx-Handler: " << std::hex << axis_tx_handler.size() << "\n"; \
    std::cout << "  Kernel-Out: " << std::hex << axis_kernel_out.size() << "\n"; \
    std::cout << "  Kernel-In: " << std::hex << axis_kernel_in.size() << "\n"; \
    std::cout << "  Tx-Kernel: " << std::hex << axis_tx_kernel.size() << "\n";

int main(int argc, char* argv[]){
    int i;
    int verbose = 0;
    for(i = 1; i < argc; i++){
        if (std::string(argv[i]).compare("-v") == 0) {
            verbose++;
        } else {
            std::cout << "Not enough or invalid arguments, please try again.\n";
            return 2;
        }
    }

    axis_t axis_tx_kernel; //output
    axis_t axis_rx; //input
    axis_t axis_tx_handler; //output
    axis_t axis_kernel_out; //output
    axis_t axis_kernel_in; //input
    uint_1_t blockingWait; //output

    counter_t AMcounter_threshold;
    counter_t wordCounter_threshold;
    counter_t customCounter_master[COUNTER_NUM];
    counter_t handlerCounter_threshold[COUNTER_NUM];
    counter_t handlerCounter_master[COUNTER_NUM];
    word_t enable;
    word_t mask;

    axis_word_t axis_word;

    uint_64_t readData;
    uint_1_t readLast;

    #ifdef DEBUG
    int dbg_currentState;
    #endif

    OPEN_FILE(testData)

    std::cout << "\n*** Starting XPAMS_TB ***\n\n";

    std::string key, id;
    uint_64_t hexData;
    uint_1_t hexLast, callEnable;
    uint_9_t keep;
    bool valid = true;
    while(testData >> key >> hexData >> hexLast >> callEnable >> keep >> id){
        bool read = false;
        if(key.compare("axis_rx") == 0){
            CHECK_DEBUG
            else{
                WRITE_WORD(axis_word, hexData, hexLast, axis_rx)
            }
        }
        else if(key.compare("axis_kernel_in") == 0){
            CHECK_DEBUG
            else{
                WRITE_WORD(axis_word, hexData, hexLast, axis_kernel_in)
            }
        }
        else if(key.compare("axis_tx_handler") == 0){
            CHECK_DEBUG
            else{
                read = true;
                READ_WORD(axis_word, readData, readLast, axis_tx_handler)
            }
        }
        else if(key.compare("axis_kernel_out") == 0){
            CHECK_DEBUG
            else{
                read = true;
                READ_WORD(axis_word, readData, readLast, axis_kernel_out)
            }
        }
        else if(key.compare("axis_tx_kernel") == 0){
            CHECK_DEBUG
            else{
                read = true;
                READ_WORD(axis_word, readData, readLast, axis_tx_kernel)
            }
        }
        else if(key.compare("END") == 0){
            if(!valid){
                std::cout << "Test " << std::hex << hexData << " failed\n";
            }
            else{
                std::cout << "Test " << std::hex << hexData << " successful\n";
            }
            valid = true;
        }
        else{
            std::cout << "Unknown key: " << key << "\n";
            return 1;
        }

        if(read){
            if(hexData != readData || hexLast != readLast){
                valid = false;
                std::cout << "Mismatch:\n";
                std::cout << std::hex << "   Expected: " << hexData << " " << 
                    hexLast << "\n";
                std::cout << std::hex << "   Received: " << readData << " " << 
                    readLast << "\n";
            }
            else if(verbose > 0){
                std::cout << "Match:\n";
                std::cout << std::hex << "   Received: " << readData << " " << 
                    readLast << "\n";
            }
        }
        else if(key.compare("END") != 0 && callEnable == 1){
            CALL_TB
        }
        //needed because of weird error in CSim
        if(key.compare("END") == 0 && callEnable == 1){
            break;
        }
    }

    std::cout << "\n*** Finishing XPAMS_TB ***\n";

    return 0;
    
}
