#include <fstream>
#include <iostream>
#include "am_rx.hpp"


#define CALL_TB am_rx(axis_handler, axis_net,axis_s2mmCommand,axis_s2mm, \
        axis_s2mmStatus, token_get, token_get_v, record, release);

#define AXIS_EMPTY axis_handler.empty() && axis_net.empty() && \
    axis_s2mmCommand.empty() && axis_s2mm.empty() && axis_s2mmStatus.empty()

#define READ_AXIS(Aaxis) i = 0; \
    readSize = Aaxis.size(); \
    while (i < readSize){ \
        if(!Aaxis.empty()){ \
            Aaxis.read(axis_word); \
            i++; \
        } \
    }

#define PRINT_AXIS std::cout << "Stream statuses:\n"; \
    std::cout << "  Handler: " << axis_handler.size() << "\n"; \
    std::cout << "  Network: " << axis_net.size() << "\n"; \
    std::cout << "  s2mmCommand: " << axis_s2mmCommand.size() << "\n"; \
    std::cout << "  s2mm: " << axis_s2mm.size() << "\n"; \
    std::cout << "  s2mmStatus: " << axis_s2mmStatus.size() << "\n";

int main(){

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

    token_get = 0xABCD;
    token_get_v = 1;

    gc_AMsrc_t src;
    gc_AMdst_t dst;
    gc_AMwords_t words;

    gc_AMtype_t type;
    gc_AMargs_t args;
    gc_AMhandler_t handler;

    int i;
    int readSize;

    uint_32_t readData;
    uint_1_t readLast;

    std::ifstream testData("am_rx.dat");
    if (!testData){
        std::cout << "Unable to open test data file\n";
        return -1;
    }

    std::string key, key_gold;
    uint_32_t hexData, hexData_gold;
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
                else{
                	if(key_gold.compare("axis_s2mmCommand") == 0){
                		//READ_WORD(axis_word, readData, readLast, &map2[key])
                		std::cout << "Unsupported stream access\n";
                        return -1;
                    }
                	else if(key_gold.compare("axis_handler") == 0){
                		READ_WORD(axis_word, readData, readLast, axis_handler)
                    }
                    else if(key_gold.compare("axis_s2mm")){
                        READ_WORD(axis_word, readData, readLast, axis_s2mm)
                    }
                    else{
                        std::cout << "ERROR in reading words\n";
                    }
                    if(hexData_gold != readData && hexLast_gold != readLast){
                        valid = false;
                        std::cout << "Expected: " << hexData_gold << " " << 
                            hexLast_gold << "\n";
                        std::cout << "Received: " << readData << " " << 
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
