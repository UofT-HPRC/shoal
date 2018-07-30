#ifndef XPAMS_H_
#define XPAMS_H_

#include "gascore.hpp"

#define DIRECT_REPLY 0
#define REPLY_HANDLER 1
#define CODE_LOAD 2

void xpams(
    axis_t &axis_handler, //input
    axis_t &axis_handler_return //output
);

bool isDirectReply(gc_AMhandler_t handler);
bool isReply(gc_AMhandler_t handler);
bool isCodeLoad(gc_AMhandler_t handler);

#endif