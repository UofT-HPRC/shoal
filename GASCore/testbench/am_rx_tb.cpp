#include <fstream>
#include <iostream>
#include <cstdlib>
#include "am_rx.hpp"

#define DAT_FILE "/GASCore/testbench/am_rx.dat" //relative to repo root

#define CALL_TB am_rx(axis_handler, axis_net,axis_s2mmCommand,axis_s2mm, \
        axis_s2mmStatus, token_get, token_get_v, record, release, dbgState);

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
        // } else if (argv[i] == "-p") {
        //     myPath = argv[i + 1];
        // } else if (argv[i] == "-o") {
        //     myOutPath = argv[i + 1];
        } else {
            std::cout << "Not enough or invalid arguments, please try again.\n";
            return 2;
        }
    }

    std::cout << "Verbosity set to level " << verbose << "\n";

    axis_32a_t axis_handler; //output
    axis_32a_t axis_net; //input
    s2mmCommand_t axis_s2mmCommand; //output
    axis_32a_t axis_s2mm; //output
    axis_32a_t axis_s2mmStatus; //input

    //token RAM
    uint_16_t token_get; //input
    uint_1_t token_get_v; //input
    uint_8_t record; //output

    //axis_handler release
    uint_1_t release; //output

    axis_word_32a_t axis_word;
    axis_word_72a_t axis_word_s2mmCommand;
    token_get = 0xABCD;
    token_get_v = 1;

    gc_AMsrc_t src;
    gc_AMdst_t dst;
    gc_AMwords_t words;

    gc_AMtype_t type;
    gc_AMargs_t args;
    gc_AMhandler_t handler;

    uint_72_t readData;
    uint_1_t readLast;

    int dbgState;

    char const* tmp_repo_path = std::getenv("SHOAL_PATH");
    if(tmp_repo_path == NULL){
        std::cout << "SHOAL_PATH not set in environment\n";
        return -1;
    }
    std::string repo_path(tmp_repo_path);
    std::ifstream testData(repo_path.append(DAT_FILE).c_str());
    if (!testData){
        std::cout << "Unable to open test data file\n";
        return -1;
    }

    std::string key, key_gold;
    uint_72_t hexData, hexData_gold;
    uint_1_t hexLast, hexLast_gold, callEnable, callEnable_gold;
    while(testData >> key >> hexData >> hexLast >> callEnable){
        if(key.compare("Token") == 0){
            token_get = hexData(15,0);
            continue;
        }
        else if(key.compare("END") == 0 && hexLast == 0){
            CALL_TB //move from done state back to initial
            bool valid = true;
            while(testData >> key_gold >> hexData_gold >> hexLast_gold >> callEnable_gold){
                if(key_gold.compare("END") == 0 && hexLast_gold == 1){
                    break;
                }
                else if(key_gold.compare("DEBUG") == 0){
                    if(hexData_gold == 0){
                        PRINT_AXIS
                    }
                    else if(hexData_gold == 1){
                        std::cout << "Current State: " << dbgState << "\n";
                    }
                }
                else{
                	if(key_gold.compare("axis_s2mmCommand") == 0){
                        axis_s2mmCommand.read(axis_word_s2mmCommand);
                        readData = axis_word_s2mmCommand.data;
                        readLast = axis_word_s2mmCommand.last;
                    }
                	else if(key_gold.compare("axis_handler") == 0){
                		READ_WORD(axis_word, readData, readLast, axis_handler)
                    }
                    else if(key_gold.compare("axis_s2mm") == 0){
                        READ_WORD(axis_word, readData, readLast, axis_s2mm)
                    }
                    else{
                        std::cout << "Unknown key: " << key_gold << "\n";
                        return 1;
                    }
                    if(hexData_gold != readData || hexLast_gold != readLast){
                        valid = false;
                        std::cout << "Mismatch:\n";
                        std::cout << std::hex << "   Expected: " << hexData_gold << " " << 
                            hexLast_gold << "\n";
                        std::cout << std::hex << "   Received: " << readData << " " << 
                            readLast << "\n";
                    }
                    else if(verbose > 0){
                        std::cout << "Match:\n";
                        std::cout << std::hex << "   Received: " << readData << " " << 
                            readLast << "\n";
                    }
                }
            }
            if(!valid){
                std::cout << "Test " << std::hex << hexData_gold << " failed\n";
            }
            else{
                std::cout << "Test " << std::hex << hexData_gold << " successful\n";
            }
        }
        else if(key.compare("DEBUG") == 0){
            if(hexData == 0){
                PRINT_AXIS
            }
            else if(hexData == 1){
                std::cout << "Current State: " << dbgState << "\n";
            }
        }
        else{
            if(key.compare("axis_net") == 0){
                WRITE_WORD(axis_word, hexData, hexLast, axis_net)
            }
            else if(key.compare("axis_s2mmStatus") == 0){
                WRITE_WORD(axis_word, hexData, hexLast, axis_s2mmStatus)
            }
            else{
                std::cout << "ERROR in writing words: key used: " << key << "\n";
                return -1;
            }
            if(callEnable == 1)
                CALL_TB
        }
        //needed because of weird error in CSim
        if(key_gold.compare("END") == 0 && callEnable_gold == 1){
            break;
        }
    }

    return 0;
    

}
