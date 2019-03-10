#if !defined(SHOAL_ACTIVE_MESSAGES_H_)
#define SHOAL_ACTIVE_MESSAGES_H_

#include "config.hpp"
#include "user_config.hpp"
#include "utilities.hpp"

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

bool isShortAM(gc_AMtype_t arg);
bool isMediumAM(gc_AMtype_t arg);
bool isLongxAM(gc_AMtype_t arg);
bool isLongAM(gc_AMtype_t arg);
bool isLongVectoredAM(gc_AMtype_t arg);
bool isLongStridedAM(gc_AMtype_t arg);
bool isDataFromFIFO(gc_AMtype_t arg);
bool isAsyncAM(gc_AMtype_t arg);
bool isReplyAM(gc_AMtype_t arg);
bool isMediumFIFOAM(gc_AMtype_t arg);
bool isLongFIFOAM(gc_AMtype_t arg);

#endif

#endif // SHOAL_ACTIVE_MESSAGES_H_
