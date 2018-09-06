#include <fstream>
#include <iostream>
#include <cstdlib>
#include "am_tx.hpp"

#define DAT_FILE "/GASCore/testbench/build/am_tx_c.dat" //relative to repo root

#ifdef DEBUG
#define CALL_TB am_tx(dbg_currentState, axis_kernel, axis_net, axis_mm2sCommand,axis_mm2s, \
        axis_mm2sStatus, release);
#else
#define CALL_TB am_tx(axis_kernel, axis_net,axis_mm2sCommand,axis_mm2s, \
        axis_mm2sStatus, release);
#endif

#define PRINT_AXIS std::cout << "Stream statuses:\n"; \
    std::cout << "  Kernel: " << axis_kernel.size() << "\n"; \
    std::cout << "  Network: " << axis_net.size() << "\n"; \
    std::cout << "  mm2sCommand: " << axis_mm2sCommand.size() << "\n"; \
    std::cout << "  mm2s: " << axis_mm2s.size() << "\n"; \
    std::cout << "  mm2sStatus: " << axis_mm2sStatus.size() << "\n";

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

    axis_t axis_net; //output
    axis_t axis_kernel; //input
    dataMoverCommand_t axis_mm2sCommand; //output
    axis_t axis_mm2s; //output
    dataMoverStatus_t axis_mm2sStatus; //input

    //axis_net release
    uint_1_t release; //output

    axis_word_t axis_word;
    dataMoverCommand_word_t axis_word_mm2sCommand;
    axis_word_8a_t axis_word_mm2sStatus;

    uint_64_t readData;
    uint_1_t readLast;

    #ifdef DEBUG
    int dbg_currentState;
    #endif

    OPEN_FILE("SHOAL_PATH",testData)

    std::cout << "\n*** Starting AM_TX_TB ***\n\n";

    std::string key, id;
    uint_64_t hexData;
    uint_1_t hexLast, callEnable;
    uint_11_t keep;
    bool valid = true;
    while(testData >> key >> hexData >> hexLast >> callEnable >> keep >> id){
        bool read = false;
        if(key.compare("axis_kernel") == 0){
            CHECK_DEBUG
            else{
                WRITE_WORD(axis_word, hexData, hexLast, axis_kernel)
            }
        }
        else if(key.compare("axis_mm2sStatus") == 0){
            CHECK_DEBUG
            else{
                WRITE_WORD(axis_word_mm2sStatus, hexData, hexLast, axis_mm2sStatus)
            }
        }
        else if(key.compare("axis_net") == 0){
            CHECK_DEBUG
            else{
                read = true;
                READ_WORD(axis_word, readData, readLast, axis_net)
            }
        }
        else if(key.compare("axis_mm2sCommand") == 0){
            CHECK_DEBUG
            else{
                read = true;
                READ_WORD(axis_word_mm2sCommand, readData, readLast, axis_mm2sCommand)
            }
        }
        else if(key.compare("axis_mm2s") == 0){
            CHECK_DEBUG
            else{
                WRITE_WORD(axis_word, hexData, hexLast, axis_mm2s)
            }
        }
        else if(key.compare("end") == 0){
            if(!valid){
                std::cout << "Test " << std::hex << hexData << " failed\n";
            }
            else{
                std::cout << "Test " << std::hex << hexData << " successful\n";
            }
            valid = true;
        }
        else if(key.compare("finish") == 0){
            break;
        }
        else{
            std::cout << "Unknown key: " << key << "\n";
            return 1;
        }

        if(read){
            if(hexData != readData || hexLast != readLast){
                valid = false;
                std::cout << "Mismatch at id: " << id << "\n";
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
        if(callEnable == 1){
            CALL_TB
        }
    }

    std::cout << "\n*** Finishing AM_TX_TB ***\n";

    return 0;
    

}
