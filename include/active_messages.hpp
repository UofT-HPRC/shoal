#ifndef ACTIVE_MESSAGES_H
#define ACTIVE_MESSAGES_H

#include "../GAScore/include/utilities.hpp"

void sendShortAM(
    gc_AMsrc_t src,
    gc_AMdst_t dst,
    gc_AMToken_t token,
    gc_AMhandler_t handlerID,
    gc_AMargs_t handlerArgCount,
    word_t * handler_args,
    axis_t & axis_out
);
// void sendMediumAM(bool isAsync);
// void sendLongAM(bool isAsync);
// void sendStridedAM(bool isAsync);
// void sendVectoredAM(bool isAsync);

#endif // ACTIVE_MESSAGES_H