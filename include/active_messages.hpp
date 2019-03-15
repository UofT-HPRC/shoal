#ifndef SHOAL_INCLUDE_ACTIVE_MESSAGES_H_
#define SHOAL_INCLUDE_ACTIVE_MESSAGES_H_

#include "config.hpp"

#if defined(__x86_64__)

/*******************************************************************************
 * Active Message constructors
*******************************************************************************/

long long createHeader(
    int src,
    int dst,
    int payloadSize,
    int handlerID,
    int type,
    int handlerArgCount
);
long long createToken(int token);
long long createVectorToken(
    int srcVectorCount,
    int dstVectorCount,
    int srcSize1,
    int dstSize1,
    int token
);
long long createStridedToken(
    int stride,
    int blockSize,
    int blockNum,
    int token
);
long long createStridedToken(
    int stride,
    int blockSize,
    int blockNum
);
long long createStridedSrcBody(int size);

/*******************************************************************************
 * Active Message type check
*******************************************************************************/

bool isShortAM(char arg);
bool isMediumAM(char arg);
bool isLongxAM(char arg);
bool isLongAM(char arg);
bool isLongVectoredAM(char arg);
bool isLongStridedAM(char arg);
bool isDataFromFIFO(char arg);
bool isAsyncAM(char arg);
bool isReplyAM(char arg);
bool isMediumFIFOAM(char arg);
bool isLongFIFOAM(char arg);

#endif

#endif // SHOAL_INCLUDE_ACTIVE_MESSAGES_H_
