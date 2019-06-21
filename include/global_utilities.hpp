#ifndef SHOAL_INCLUDE_GLOBAL_UTILITIES_H_
#define SHOAL_INCLUDE_GLOBAL_UTILITIES_H_

/*******************************************************************************
 * This allows you to use defined variables as part of pragmas.
 * i.e. ADD_PRAGMA(HLS STREAM depth=FIFO_DEPTH variable=axis_fifo) expands to
 * #pragma HLS STREAM depth=512 variable=axis_fifo where the 512 may be the 
 * defined value of FIFO_DEPTH
*******************************************************************************/

#define ADD_PRAGMA_INNER(x) _Pragma (#x)
#define ADD_PRAGMA(x) ADD_PRAGMA_INNER(x)

/*******************************************************************************
 * This allows macros to be expanded differently based on the values of the 
 * arguments. i.e. 
 * 
 * #define COMPARE_foo(x) x
 * #define COMPARE_bar(x) x
 * #define WRITE(key) \
    WHEN(EQUAL(key, foo))(WRITE_0)\
    WHEN(EQUAL(key, bar))(READ_0)
 * ^ Here, the macro WRITE(key) will expand to another macro WRITE_0 if key is
 * equal to foo or to READ_0 if it is equal to bar. The COMPARE_xxx defines 
 * need to be specified for all options and therefore other define statements 
 * cannot start with COMPARE_
 * 
 * Reference: https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms
 * Credit to Paul Fultz II and Cloak (https://github.com/pfultz2/Cloak)
*******************************************************************************/

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define IIF(c) PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...) __VA_ARGS__
#define IIF_1(t, ...) t

#define COMPL(b) PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0

#define BITAND(x) PRIMITIVE_CAT(BITAND_, x)
#define BITAND_0(y) 0
#define BITAND_1(y) y

#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x) x, 1,

#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)
#define IS_PAREN_PROBE(...) PROBE(~)

#define NOT(x) CHECK(PRIMITIVE_CAT(NOT_, x))
#define NOT_0 PROBE(~)

#define BOOL(x) COMPL(NOT(x))
#define IF(c) IIF(BOOL(c))

#define EAT(...)
#define EXPAND(...) __VA_ARGS__
#define WHEN(c) IF(c)(EXPAND, EAT)

#define EXPAND(...) __VA_ARGS__

#define PRIMITIVE_COMPARE(x, y) IS_PAREN \
    ( COMPARE_ ## x ( COMPARE_ ## y) (()) )

#define IS_COMPARABLE(x) IS_PAREN( CAT(COMPARE_, x) (()) )

#define NOT_EQUAL(x, y) \
    IIF(BITAND(IS_COMPARABLE(x))(IS_COMPARABLE(y)) ) ( \
        PRIMITIVE_COMPARE, 1 EAT \
    )(x, y)

#define EQUAL(x, y) COMPL(NOT_EQUAL(x, y))

// #define EMPTY()
// #define DEFER(id) id EMPTY()
// #define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()

// #define WHILE(pred, op, ...) \
//     IF(pred(__VA_ARGS__)) \
//     ( \
//         OBSTRUCT(WHILE_INDIRECT) () \
//         ( \
//             pred, op, op(__VA_ARGS__) \
//         ), \
//         __VA_ARGS__ \
//     )
// #define WHILE_INDIRECT() WHILE

// #define DEC(x) PRIMITIVE_CAT(DEC_, x)
// #define DEC_0 0
// #define DEC_1 0
// #define DEC_2 1
// #define DEC_3 2
// #define DEC_4 3
// #define DEC_5 4
// #define DEC_6 5
// #define DEC_7 6
// #define DEC_8 7
// #define DEC_9 8

// #define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
// #define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
// #define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
// #define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
// #define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
// #define EVAL5(...) __VA_ARGS__

// #define REPEAT(count, macro, ...) \
//     WHEN(count) \
//     ( \
//         OBSTRUCT(REPEAT_INDIRECT) () \
//         ( \
//             DEC(count), macro, __VA_ARGS__ \
//         ) \
//         OBSTRUCT(macro) \
//         ( \
//             DEC(count), __VA_ARGS__ \
//         ) \
//     )
// #define REPEAT_INDIRECT() REPEAT

#define NBITS2(n) ((n&2)?1:0)
#define NBITS4(n) ((n&(0xC))?(2+NBITS2(n>>2)):(NBITS2(n)))
#define NBITS8(n) ((n&0xF0)?(4+NBITS4(n>>4)):(NBITS4(n)))
#define NBITS16(n) ((n&0xFF00)?(8+NBITS8(n>>8)):(NBITS8(n)))
#define NBITS32(n) ((n&0xFFFF0000)?(16+NBITS16(n>>16)):(NBITS16(n)))
#define NBITS64(n) ((n&0xFFFFFFFF0000)?(32+NBITS32(n>>32)):(NBITS32(n)))
#define NBITS(n) (n==0?0:NBITS64(n)+1)

template<int base, int exponent>
int power() {
    int sum = 1;
    for(int i = 0; i < exponent; i++)
        sum *= base;
    
    return sum;
}

template<unsigned long long base, int exponent>
unsigned long long power_64() {
    unsigned long long sum = 1ULL;
    for(int i = 0; i < exponent; i++)
        sum *= base;
    
    return sum;
}

#define POWER_2(x) (1ULL << (x))

// taken from https://stackoverflow.com/a/17469726
namespace Color {
    enum Code {
        BOLD_ON     = 1,
        BOLD_OFF    = 21,
        FG_RED      = 31,
        FG_GREEN    = 32,
        FG_BLUE     = 34,
        FG_DEFAULT  = 39,
        BG_RED      = 41,
        BG_GREEN    = 42,
        BG_BLUE     = 44,
        BG_DEFAULT  = 49
    };
    class Modifier {
        Code code;
    public:
        Modifier(Code pCode) : code(pCode) {}
        friend std::ostream&
        operator<<(std::ostream& os, const Modifier& mod) {
            return os << std::dec << "\033[" << mod.code << "m";
        }
    };
}

#define _COLOR(color, mode) std::dec << "\033[" << color << "m" << std::mode

#define COLOR(color, mode, data) _COLOR(color, mode) << data << _COLOR(0, mode)

#define hdextract(arg, bits) (((arg) & bits##_BITMASK) >> bits##_LOWER)
#define hdencode(arg, bits,data) (((arg) & (~bits##_BITMASK)) | (((long long)(data) << bits##_LOWER) & bits##_BITMASK))

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#endif // SHOAL_INCLUDE_GLOBAL_UTILITIES_H_
