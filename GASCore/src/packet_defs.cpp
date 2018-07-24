#include "packet_defs.hpp"

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

bool isFIFOnotMemData(gc_AMtype_t arg){
    return arg[4] == 1;
}

bool isAsyncAM(gc_AMtype_t arg){
    return arg[5] == 1;
}

bool isReplyAM(gc_AMtype_t arg){
    return arg[6] == 1;
}