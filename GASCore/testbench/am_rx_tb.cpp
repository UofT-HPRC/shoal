#include <fstream>
#include <iostream>
#include <cstdlib>
#include "am_rx.hpp"
#include "testbench.hpp"

#define DAT_FILE "/GASCore/testbench/build/am_rx.dat" //relative to repo root

#ifdef DEBUG
#define CALL_TB am_rx(dbg_currentState, axis_handler, axis_net,axis_s2mmCommand,axis_s2mm, \
        axis_s2mmStatus, release);
#else
#define CALL_TB am_rx(axis_handler, axis_net,axis_s2mmCommand,axis_s2mm, \
        axis_s2mmStatus, release);
#endif

#define PRINT_AXIS std::cout << "Stream statuses:\n"; \
    std::cout << "  Handler: " << axis_handler.size() << "\n"; \
    std::cout << "  Network: " << axis_net.size() << "\n"; \
    std::cout << "  s2mmCommand: " << axis_s2mmCommand.size() << "\n"; \
    std::cout << "  s2mm: " << axis_s2mm.size() << "\n"; \
    std::cout << "  s2mmStatus: " << axis_s2mmStatus.size() << "\n";

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

    axis_t axis_handler; //output
    axis_t axis_net; //input
    dataMoverCommand_t axis_s2mmCommand; //output
    axis_t axis_s2mm; //output
    dataMoverStatus_t axis_s2mmStatus; //input

    //axis_handler release
    uint_1_t release; //output

    axis_word_t axis_word;
    dataMoverCommand_word_t axis_word_s2mmCommand;
    axis_word_8a_t axis_word_s2mmStatus;

    uint_64_t readData;
    uint_1_t readLast;

    #ifdef DEBUG
    int dbg_currentState;
    #endif

    OPEN_FILE(testData)

    std::cout << "\n*** Starting AM_RX_TB ***\n\n";

    std::string key, id;
    uint_64_t hexData;
    uint_1_t hexLast, callEnable;
    uint_9_t keep;
    bool valid = true;
    while(testData >> key >> hexData >> hexLast >> callEnable >> keep >> id){
        bool read = false;
        if(key.compare("axis_net") == 0){
            CHECK_DEBUG
            else{
                WRITE_WORD(axis_word, hexData, hexLast, axis_net)
            }
        }
        else if(key.compare("axis_s2mmStatus") == 0){
            CHECK_DEBUG
            else{
                WRITE_WORD(axis_word_s2mmStatus, hexData, hexLast, axis_s2mmStatus)
            }
        }
        else if(key.compare("axis_handler") == 0){
            CHECK_DEBUG
            else{
                read = true;
                READ_WORD(axis_word, readData, readLast, axis_handler)
            }
        }
        else if(key.compare("axis_s2mmCommand") == 0){
            CHECK_DEBUG
            else{
                read = true;
                READ_WORD(axis_word_s2mmCommand, readData, readLast, axis_s2mmCommand)
            }
        }
        else if(key.compare("axis_s2mm") == 0){
            CHECK_DEBUG
            else{
                read = true;
                READ_WORD(axis_word, readData, readLast, axis_s2mm)
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
                    hexLast << "id: " << id << "\n";
                std::cout << std::hex << "   Received: " << readData << " " << 
                    readLast << "id: " << id << "\n";
            }
            else if(verbose > 0){
                std::cout << "Match:\n";
                std::cout << std::hex << "   Received: " << readData << " " << 
                    readLast << "id: " << id << "\n";
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

    std::cout << "\n*** Finishing AM_RX_TB ***\n";

    return 0;
    

}
