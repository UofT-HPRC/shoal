#ifndef ACTIVE_MESSAGES_H
#define ACTIVE_MESSAGES_H

#include "../share/include/shoal_stream.hpp"
#include "../GAScore/include/GAScore.hpp"

void sendShortAM(gc_AMsrc_t src, gc_AMdst_t dst, gc_AMToken_t token, 
    gc_AMhandler_t handler_ID, gc_AMargs_t handler_argCount,  
    gc_AMhandlerArg_t * handler_args);
void sendMediumAM(bool isAsync);
void sendLongAM(bool isAsync);
void sendStridedAM(bool isAsync);
void sendVectoredAM(bool isAsync);

#endif // ACTIVE_MESSAGES_H
