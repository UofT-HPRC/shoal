#include "utilities.hpp"

bool isShortAM(gc_AMtype_t arg){
    return arg(2,0) == 0b001;
}

bool isMediumAM(gc_AMtype_t arg){
    return arg(2,0) == 0b010;
}

bool isLongxAM(gc_AMtype_t arg){
    return arg(2,2) == 1;
}

bool isLongAM(gc_AMtype_t arg){
    return arg(2,0) == 0b100;
}

bool isLongVectoredAM(gc_AMtype_t arg){
    return arg(2,0) == 0b110;
}

bool isLongStridedAM(gc_AMtype_t arg){
    return arg(2,0) == 0b101;
}

bool isDataFromFIFO(gc_AMtype_t arg){
    return arg[4] == 1;
}

bool isAsyncAM(gc_AMtype_t arg){
    return arg[5] == 1;
}

bool isReplyAM(gc_AMtype_t arg){
    return arg[6] == 1;
}

bool isMediumFIFOAM(gc_AMtype_t arg){
    return isMediumAM(arg) && isDataFromFIFO(arg);
}

bool isLongFIFOAM(gc_AMtype_t arg){
    return isLongAM(arg) && isDataFromFIFO(arg);
}

void dataMoverWriteCommand(
    dataMoverCommand_t &axis_command, //output
    uint_4_t reserved,
    uint_4_t tag,
    addr_word_t address,
    uint_1_t ddr,
    uint_1_t eof,
    uint_6_t dsa,
    uint_1_t type,
    btt_t btt
){
    #define tmp GC_MAX_PAYLOAD_BYTES
    #define temp (tmp + GC_ADDR_WIDTH)
    dataMoverCommand_word_t axis_word_s2mmCommand;
    axis_word_s2mmCommand.data(temp+16,temp+13) = reserved;
    axis_word_s2mmCommand.data(temp+12,temp+9) = tag;
    axis_word_s2mmCommand.data(temp+8,tmp+9) = address;
    axis_word_s2mmCommand.data[tmp+8] = ddr;
    axis_word_s2mmCommand.data[tmp+7] = eof;
    axis_word_s2mmCommand.data(tmp+6, tmp+1) = dsa;
    axis_word_s2mmCommand.data[tmp] = type;
    axis_word_s2mmCommand.data(tmp-1,0) = btt;
    axis_word_s2mmCommand.data(tmp-1,0) = btt;
    axis_word_s2mmCommand.keep = 0xFF; //! parameterize
    if(!axis_command.full()){
        axis_command.write(axis_word_s2mmCommand);
    }
    #undef temp
    #undef tmp
}
