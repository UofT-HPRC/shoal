#ifndef UTILITIES_H_
#define UTILITIES_H_

/*******************************************************************************
 * Includes
*******************************************************************************/

#include "shoal_utilities.hpp"

/*******************************************************************************
 * Macros
*******************************************************************************/

#define COMPARE_uaxis_l(x) x
#define COMPARE_uaxis_m(x) x

#define WRITE_WORD_L(Aword, Adata, Alast, Akeep, Aaxis)\
    Aword.data = Adata;\
    Aword.last = Alast;\
    Aword.keep = Akeep;\
    Aaxis.write(Aword);

#define WRITE_WORD_M(Aword, Adata, Alast, Akeep, Adest, Aaxis)\
    Aword.data = Adata;\
    Aword.last = Alast;\
    Aword.keep = Akeep;\
    Aword.dest = Adest;\
    Aaxis.write(Aword);

#define READ_WORD_L(Aword, Adata, Alast, Akeep, Aaxis)\
    Aaxis.read(Aword);\
    Adata = Aword.data;\
    Alast = Aword.last;\
    Akeep = Aword.keep;

#define READ_WORD_M(Aword, Adata, Alast, Akeep, Adest, Aaxis)\
    Aaxis.read(Aword);\
    Adata = Aword.data;\
    Alast = Aword.last;\
    Akeep = Aword.keep;\
    Adest = Aword.dest;

//the arg numbers correspond to the c_args order in axis.py in Sonar
#define WRITE(key, word_type, interface)\
    WHEN(EQUAL(key, uaxis_l))\
        (WRITE_WORD_L(word_type, args[0], args[1], args[2], interface))\
    WHEN(EQUAL(key, uaxis_m))\
    	(WRITE_WORD_M(word_type, args[0], args[1], args[2], args[3], interface))

#define READ(key, word_type, interface)\
    WHEN(EQUAL(key, uaxis_l))\
    	(READ_WORD_L(word_type, readArgs[0], readArgs[1], readArgs[2], interface))\
    WHEN(EQUAL(key, uaxis_m))\
    	(READ_WORD_M(word_type, readArgs[0], readArgs[1], readArgs[2], readArgs[3], interface))

#define VERIFY(key)\
    if(!strcmp(key,"uaxis_l")){\
        if(args[0] != readArgs[0] || args[1] != readArgs[1] || \
        args[2] != readArgs[2]){\
            valid = false;\
            std::cout << "Mismatch at id: " << id << "\n";\
            std::cout << std::hex << "   Expected: " << args[0] << " " << \
                args[1] << " " << args[2] << "\n";\
            std::cout << std::hex << "   Received: " << readArgs[0] << " " << \
                readArgs[1] << " " << readArgs[2] << "\n";\
        }\
    }\
    else if(!strcmp(key,"uaxis_m")){\
        if(args[0] != readArgs[0] || args[1] != readArgs[1] || \
        args[2] != readArgs[2] || args[3] != readArgs[3]){\
            valid = false;\
            std::cout << "Mismatch at id: " << id << "\n";\
            std::cout << std::hex << "   Expected: " << args[0] << " " << \
                args[1] << " " << args[2] << " " << args[3] << "\n";\
            std::cout << std::hex << "   Received: " << readArgs[0] << " " << \
                readArgs[1] << " " << readArgs[2] << " " << readArgs[3] << "\n";\
        }\
    }

#define READ_STREAM_INTERFACE(name, key, stream, word_type) \
    while(stream.size() > 0){ \
        WHEN(EQUAL(key, uaxis_l))(\
            READ(key, word_type, stream) \
            std::cout << std::hex << name << " - Data: " << word_type.data << \
            " Last: " << word_type.last << " keep: " << word_type.keep << "\n"; \
        )\
        WHEN(EQUAL(key, uaxis_m))(\
            READ(key, word_type, stream) \
            std::cout << std::hex << name << " - Data: " << word_type.data << \
            " Last: " << word_type.last << " Keep: " << word_type.keep << \
            " Dest: " << word_type.dest << "\n"; \
        )\
    }

/*******************************************************************************
 * Function Prototypes
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
);

#endif