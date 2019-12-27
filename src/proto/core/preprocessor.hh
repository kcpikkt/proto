#pragma once

#define VARIADIC_SIZE_I(_1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9,  _10, _11, _12, _13, _14, _15, _16, \
                                 _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, \
                                 _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, \
                                 _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, size, ...) size

#define VARIADIC_SIZE(...)                                     \
    VARIADIC_SIZE_I(__VA_ARGS__ __VA_OPT__(,)                  \
                             64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,    \
                             48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,    \
                             32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,    \
                             16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1, 0)

#define CONCAT_DETAIL1(A,B) A##B
#define CONCAT_DETAIL0(A,B) CONCAT_DETAIL1(A,B)
#define CONCAT(A,B) CONCAT_DETAIL0(A,B)

// for i in $(seq 64); do
// echo "#define FOR_EACH_$i(func, arg, ...) func(arg); FOR_EACH_$((i-1))(func, __VA_ARGS__)"; done

#define FOR_EACH_1(func, arg, ...)  func(arg)
#define FOR_EACH_2(func, arg, ...)  func(arg), FOR_EACH_1(func, __VA_ARGS__)
#define FOR_EACH_3(func, arg, ...)  func(arg), FOR_EACH_2(func, __VA_ARGS__)
#define FOR_EACH_4(func, arg, ...)  func(arg), FOR_EACH_3(func, __VA_ARGS__)
#define FOR_EACH_5(func, arg, ...)  func(arg), FOR_EACH_4(func, __VA_ARGS__)
#define FOR_EACH_6(func, arg, ...)  func(arg), FOR_EACH_5(func, __VA_ARGS__)
#define FOR_EACH_7(func, arg, ...)  func(arg), FOR_EACH_6(func, __VA_ARGS__)
#define FOR_EACH_8(func, arg, ...)  func(arg), FOR_EACH_7(func, __VA_ARGS__)
#define FOR_EACH_9(func, arg, ...)  func(arg), FOR_EACH_8(func, __VA_ARGS__)
#define FOR_EACH_10(func, arg, ...) func(arg), FOR_EACH_9(func, __VA_ARGS__)
#define FOR_EACH_11(func, arg, ...) func(arg), FOR_EACH_10(func, __VA_ARGS__)
#define FOR_EACH_12(func, arg, ...) func(arg), FOR_EACH_11(func, __VA_ARGS__)
#define FOR_EACH_13(func, arg, ...) func(arg), FOR_EACH_12(func, __VA_ARGS__)
#define FOR_EACH_14(func, arg, ...) func(arg), FOR_EACH_13(func, __VA_ARGS__)
#define FOR_EACH_15(func, arg, ...) func(arg), FOR_EACH_14(func, __VA_ARGS__)
#define FOR_EACH_16(func, arg, ...) func(arg), FOR_EACH_15(func, __VA_ARGS__)
#define FOR_EACH_17(func, arg, ...) func(arg), FOR_EACH_16(func, __VA_ARGS__)
#define FOR_EACH_18(func, arg, ...) func(arg), FOR_EACH_17(func, __VA_ARGS__)
#define FOR_EACH_19(func, arg, ...) func(arg), FOR_EACH_18(func, __VA_ARGS__)
#define FOR_EACH_20(func, arg, ...) func(arg), FOR_EACH_19(func, __VA_ARGS__)
#define FOR_EACH_21(func, arg, ...) func(arg), FOR_EACH_20(func, __VA_ARGS__)
#define FOR_EACH_22(func, arg, ...) func(arg), FOR_EACH_21(func, __VA_ARGS__)
#define FOR_EACH_23(func, arg, ...) func(arg), FOR_EACH_22(func, __VA_ARGS__)
#define FOR_EACH_24(func, arg, ...) func(arg), FOR_EACH_23(func, __VA_ARGS__)
#define FOR_EACH_25(func, arg, ...) func(arg), FOR_EACH_24(func, __VA_ARGS__)
#define FOR_EACH_26(func, arg, ...) func(arg), FOR_EACH_25(func, __VA_ARGS__)
#define FOR_EACH_27(func, arg, ...) func(arg), FOR_EACH_26(func, __VA_ARGS__)
#define FOR_EACH_28(func, arg, ...) func(arg), FOR_EACH_27(func, __VA_ARGS__)
#define FOR_EACH_29(func, arg, ...) func(arg), FOR_EACH_28(func, __VA_ARGS__)
#define FOR_EACH_30(func, arg, ...) func(arg), FOR_EACH_29(func, __VA_ARGS__)
#define FOR_EACH_31(func, arg, ...) func(arg), FOR_EACH_30(func, __VA_ARGS__)
#define FOR_EACH_32(func, arg, ...) func(arg), FOR_EACH_31(func, __VA_ARGS__)
#define FOR_EACH_33(func, arg, ...) func(arg), FOR_EACH_32(func, __VA_ARGS__)
#define FOR_EACH_34(func, arg, ...) func(arg), FOR_EACH_33(func, __VA_ARGS__)
#define FOR_EACH_35(func, arg, ...) func(arg), FOR_EACH_34(func, __VA_ARGS__)
#define FOR_EACH_36(func, arg, ...) func(arg), FOR_EACH_35(func, __VA_ARGS__)
#define FOR_EACH_37(func, arg, ...) func(arg), FOR_EACH_36(func, __VA_ARGS__)
#define FOR_EACH_38(func, arg, ...) func(arg), FOR_EACH_37(func, __VA_ARGS__)
#define FOR_EACH_39(func, arg, ...) func(arg), FOR_EACH_38(func, __VA_ARGS__)
#define FOR_EACH_40(func, arg, ...) func(arg), FOR_EACH_39(func, __VA_ARGS__)
#define FOR_EACH_41(func, arg, ...) func(arg), FOR_EACH_40(func, __VA_ARGS__)
#define FOR_EACH_42(func, arg, ...) func(arg), FOR_EACH_41(func, __VA_ARGS__)
#define FOR_EACH_43(func, arg, ...) func(arg), FOR_EACH_42(func, __VA_ARGS__)
#define FOR_EACH_44(func, arg, ...) func(arg), FOR_EACH_43(func, __VA_ARGS__)
#define FOR_EACH_45(func, arg, ...) func(arg), FOR_EACH_44(func, __VA_ARGS__)
#define FOR_EACH_46(func, arg, ...) func(arg), FOR_EACH_45(func, __VA_ARGS__)
#define FOR_EACH_47(func, arg, ...) func(arg), FOR_EACH_46(func, __VA_ARGS__)
#define FOR_EACH_48(func, arg, ...) func(arg), FOR_EACH_47(func, __VA_ARGS__)
#define FOR_EACH_49(func, arg, ...) func(arg), FOR_EACH_48(func, __VA_ARGS__)
#define FOR_EACH_50(func, arg, ...) func(arg), FOR_EACH_49(func, __VA_ARGS__)
#define FOR_EACH_51(func, arg, ...) func(arg), FOR_EACH_50(func, __VA_ARGS__)
#define FOR_EACH_52(func, arg, ...) func(arg), FOR_EACH_51(func, __VA_ARGS__)
#define FOR_EACH_53(func, arg, ...) func(arg), FOR_EACH_52(func, __VA_ARGS__)
#define FOR_EACH_54(func, arg, ...) func(arg), FOR_EACH_53(func, __VA_ARGS__)
#define FOR_EACH_55(func, arg, ...) func(arg), FOR_EACH_54(func, __VA_ARGS__)
#define FOR_EACH_56(func, arg, ...) func(arg), FOR_EACH_55(func, __VA_ARGS__)
#define FOR_EACH_57(func, arg, ...) func(arg), FOR_EACH_56(func, __VA_ARGS__)
#define FOR_EACH_58(func, arg, ...) func(arg), FOR_EACH_57(func, __VA_ARGS__)
#define FOR_EACH_59(func, arg, ...) func(arg), FOR_EACH_58(func, __VA_ARGS__)
#define FOR_EACH_60(func, arg, ...) func(arg), FOR_EACH_59(func, __VA_ARGS__)
#define FOR_EACH_61(func, arg, ...) func(arg), FOR_EACH_60(func, __VA_ARGS__)
#define FOR_EACH_62(func, arg, ...) func(arg), FOR_EACH_61(func, __VA_ARGS__)
#define FOR_EACH_63(func, arg, ...) func(arg), FOR_EACH_62(func, __VA_ARGS__)
#define FOR_EACH_64(func, arg, ...) func(arg), FOR_EACH_63(func, __VA_ARGS__)
#define FOR_EACH(func, ...) \
    CONCAT(FOR_EACH_, VARIADIC_SIZE(__VA_ARGS__))(func, __VA_ARGS__)

#define FOR_EACH_EXP_1(func, arg, ...)  func(arg);
#define FOR_EACH_EXP_2(func, arg, ...)  func(arg); FOR_EACH_EXP_1(func, __VA_ARGS__)
#define FOR_EACH_EXP_3(func, arg, ...)  func(arg); FOR_EACH_EXP_2(func, __VA_ARGS__)
#define FOR_EACH_EXP_4(func, arg, ...)  func(arg); FOR_EACH_EXP_3(func, __VA_ARGS__)
#define FOR_EACH_EXP_5(func, arg, ...)  func(arg); FOR_EACH_EXP_4(func, __VA_ARGS__)
#define FOR_EACH_EXP_6(func, arg, ...)  func(arg); FOR_EACH_EXP_5(func, __VA_ARGS__)
#define FOR_EACH_EXP_7(func, arg, ...)  func(arg); FOR_EACH_EXP_6(func, __VA_ARGS__)
#define FOR_EACH_EXP_8(func, arg, ...)  func(arg); FOR_EACH_EXP_7(func, __VA_ARGS__)
#define FOR_EACH_EXP_9(func, arg, ...)  func(arg); FOR_EACH_EXP_8(func, __VA_ARGS__)
#define FOR_EACH_EXP_10(func, arg, ...) func(arg); FOR_EACH_EXP_9(func, __VA_ARGS__)
#define FOR_EACH_EXP_11(func, arg, ...) func(arg); FOR_EACH_EXP_10(func, __VA_ARGS__)
#define FOR_EACH_EXP_12(func, arg, ...) func(arg); FOR_EACH_EXP_11(func, __VA_ARGS__)
#define FOR_EACH_EXP_13(func, arg, ...) func(arg); FOR_EACH_EXP_12(func, __VA_ARGS__)
#define FOR_EACH_EXP_14(func, arg, ...) func(arg); FOR_EACH_EXP_13(func, __VA_ARGS__)
#define FOR_EACH_EXP_15(func, arg, ...) func(arg); FOR_EACH_EXP_14(func, __VA_ARGS__)
#define FOR_EACH_EXP_16(func, arg, ...) func(arg); FOR_EACH_EXP_15(func, __VA_ARGS__)
#define FOR_EACH_EXP_17(func, arg, ...) func(arg); FOR_EACH_EXP_16(func, __VA_ARGS__)
#define FOR_EACH_EXP_18(func, arg, ...) func(arg); FOR_EACH_EXP_17(func, __VA_ARGS__)
#define FOR_EACH_EXP_19(func, arg, ...) func(arg); FOR_EACH_EXP_18(func, __VA_ARGS__)
#define FOR_EACH_EXP_20(func, arg, ...) func(arg); FOR_EACH_EXP_19(func, __VA_ARGS__)
#define FOR_EACH_EXP_21(func, arg, ...) func(arg); FOR_EACH_EXP_20(func, __VA_ARGS__)
#define FOR_EACH_EXP_22(func, arg, ...) func(arg); FOR_EACH_EXP_21(func, __VA_ARGS__)
#define FOR_EACH_EXP_23(func, arg, ...) func(arg); FOR_EACH_EXP_22(func, __VA_ARGS__)
#define FOR_EACH_EXP_24(func, arg, ...) func(arg); FOR_EACH_EXP_23(func, __VA_ARGS__)
#define FOR_EACH_EXP_25(func, arg, ...) func(arg); FOR_EACH_EXP_24(func, __VA_ARGS__)
#define FOR_EACH_EXP_26(func, arg, ...) func(arg); FOR_EACH_EXP_25(func, __VA_ARGS__)
#define FOR_EACH_EXP_27(func, arg, ...) func(arg); FOR_EACH_EXP_26(func, __VA_ARGS__)
#define FOR_EACH_EXP_28(func, arg, ...) func(arg); FOR_EACH_EXP_27(func, __VA_ARGS__)
#define FOR_EACH_EXP_29(func, arg, ...) func(arg); FOR_EACH_EXP_28(func, __VA_ARGS__)
#define FOR_EACH_EXP_30(func, arg, ...) func(arg); FOR_EACH_EXP_29(func, __VA_ARGS__)
#define FOR_EACH_EXP_31(func, arg, ...) func(arg); FOR_EACH_EXP_30(func, __VA_ARGS__)
#define FOR_EACH_EXP_32(func, arg, ...) func(arg); FOR_EACH_EXP_31(func, __VA_ARGS__)
#define FOR_EACH_EXP_33(func, arg, ...) func(arg); FOR_EACH_EXP_32(func, __VA_ARGS__)
#define FOR_EACH_EXP_34(func, arg, ...) func(arg); FOR_EACH_EXP_33(func, __VA_ARGS__)
#define FOR_EACH_EXP_35(func, arg, ...) func(arg); FOR_EACH_EXP_34(func, __VA_ARGS__)
#define FOR_EACH_EXP_36(func, arg, ...) func(arg); FOR_EACH_EXP_35(func, __VA_ARGS__)
#define FOR_EACH_EXP_37(func, arg, ...) func(arg); FOR_EACH_EXP_36(func, __VA_ARGS__)
#define FOR_EACH_EXP_38(func, arg, ...) func(arg); FOR_EACH_EXP_37(func, __VA_ARGS__)
#define FOR_EACH_EXP_39(func, arg, ...) func(arg); FOR_EACH_EXP_38(func, __VA_ARGS__)
#define FOR_EACH_EXP_40(func, arg, ...) func(arg); FOR_EACH_EXP_39(func, __VA_ARGS__)
#define FOR_EACH_EXP_41(func, arg, ...) func(arg); FOR_EACH_EXP_40(func, __VA_ARGS__)
#define FOR_EACH_EXP_42(func, arg, ...) func(arg); FOR_EACH_EXP_41(func, __VA_ARGS__)
#define FOR_EACH_EXP_43(func, arg, ...) func(arg); FOR_EACH_EXP_42(func, __VA_ARGS__)
#define FOR_EACH_EXP_44(func, arg, ...) func(arg); FOR_EACH_EXP_43(func, __VA_ARGS__)
#define FOR_EACH_EXP_45(func, arg, ...) func(arg); FOR_EACH_EXP_44(func, __VA_ARGS__)
#define FOR_EACH_EXP_46(func, arg, ...) func(arg); FOR_EACH_EXP_45(func, __VA_ARGS__)
#define FOR_EACH_EXP_47(func, arg, ...) func(arg); FOR_EACH_EXP_46(func, __VA_ARGS__)
#define FOR_EACH_EXP_48(func, arg, ...) func(arg); FOR_EACH_EXP_47(func, __VA_ARGS__)
#define FOR_EACH_EXP_49(func, arg, ...) func(arg); FOR_EACH_EXP_48(func, __VA_ARGS__)
#define FOR_EACH_EXP_50(func, arg, ...) func(arg); FOR_EACH_EXP_49(func, __VA_ARGS__)
#define FOR_EACH_EXP_51(func, arg, ...) func(arg); FOR_EACH_EXP_50(func, __VA_ARGS__)
#define FOR_EACH_EXP_52(func, arg, ...) func(arg); FOR_EACH_EXP_51(func, __VA_ARGS__)
#define FOR_EACH_EXP_53(func, arg, ...) func(arg); FOR_EACH_EXP_52(func, __VA_ARGS__)
#define FOR_EACH_EXP_54(func, arg, ...) func(arg); FOR_EACH_EXP_53(func, __VA_ARGS__)
#define FOR_EACH_EXP_55(func, arg, ...) func(arg); FOR_EACH_EXP_54(func, __VA_ARGS__)
#define FOR_EACH_EXP_56(func, arg, ...) func(arg); FOR_EACH_EXP_55(func, __VA_ARGS__)
#define FOR_EACH_EXP_57(func, arg, ...) func(arg); FOR_EACH_EXP_56(func, __VA_ARGS__)
#define FOR_EACH_EXP_58(func, arg, ...) func(arg); FOR_EACH_EXP_57(func, __VA_ARGS__)
#define FOR_EACH_EXP_59(func, arg, ...) func(arg); FOR_EACH_EXP_58(func, __VA_ARGS__)
#define FOR_EACH_EXP_60(func, arg, ...) func(arg); FOR_EACH_EXP_59(func, __VA_ARGS__)
#define FOR_EACH_EXP_61(func, arg, ...) func(arg); FOR_EACH_EXP_60(func, __VA_ARGS__)
#define FOR_EACH_EXP_62(func, arg, ...) func(arg); FOR_EACH_EXP_61(func, __VA_ARGS__)
#define FOR_EACH_EXP_63(func, arg, ...) func(arg); FOR_EACH_EXP_62(func, __VA_ARGS__)
#define FOR_EACH_EXP_64(func, arg, ...) func(arg); FOR_EACH_EXP_63(func, __VA_ARGS__)
#define FOR_EACH_EXP(func, ...) \
    CONCAT(FOR_EACH_EXP_, VARIADIC_SIZE(__VA_ARGS__))(func, __VA_ARGS__)

#define DETAIL_REFL_FIELD_TYPE(...) DETAIL_REFL_VARIADIC_ARG0(__VA_ARGS__)
#define DETAIL_REFL_FIELD_NAME(...) DETAIL_REFL_VARIADIC_ARG1(__VA_ARGS__)
#define DETAIL_REFL_SEQ_HEAD_PROBE(...) (__VA_ARGS__),
#define DETAIL_REFL_VARIADIC_ARG0(x, ...) x
#define DETAIL_REFL_VARIADIC_ARG1(_1, x, ...) x
#define DETAIL_REFL_VARIADIC_ARG2(_1, _2, x, ...) x

#define SEQ_HEAD_PROBE(...) (__VA_ARGS__),

#define IDENTITY(...) __VA_ARGS__

#define DETAIL_SEQ_REM_HEAD_BASE(head, ...) __VA_ARGS__
#define DETAIL_SEQ_REM_HEAD(...) DETAIL_SEQ_REM_HEAD_BASE(__VA_ARGS__)
#define SEQ_REM_HEAD(seq) DETAIL_SEQ_REM_HEAD(SEQ_HEAD_PROBE seq)

#define DETAIL_SEQ_GET_HEAD_BASE(head, ...) head
#define DETAIL_SEQ_GET_HEAD(...) DETAIL_SEQ_GET_HEAD_BASE(__VA_ARGS__)
#define SEQ_GET_HEAD(seq) IF(IS_NOT_EMPTY(seq), DETAIL_SEQ_GET_HEAD(SEQ_HEAD_PROBE seq),)

#define DETAIL_IS_EMPTY_0() 0
#define DETAIL_IS_EMPTY() 1 
#define IS_EMPTY(...) CONCAT(DETAIL_IS_EMPTY, __VA_OPT__(_0))()
#define IS_NOT_EMPTY(...) NOT(IS_EMPTY(__VA_ARGS__))

#define DETAIL_NOT_1() 0
#define DETAIL_NOT_0() 1
#define NOT(VAL) CONCAT(DETAIL_NOT_, VAL)()

#define REM_PAREN(...) IDENTITY __VA_ARGS__

#define DETAIL_IF_1(T, F) T
#define DETAIL_IF_0(T, F) F
#define IF(VAL, T, F) CONCAT(DETAIL_IF_, VAL)(T, F)


//#define SEQ_TO_LIST_1(seq)  REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    SEQ_GET_HEAD(seq), () ))
//#define SEQ_TO_LIST_2(seq)  REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_1(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_3(seq)  REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_2(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_4(seq)  REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_3(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_5(seq)  REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_4(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_6(seq)  REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_5(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_7(seq)  REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_6(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_8(seq)  REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_7(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_9(seq)  REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_8(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_10(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_9(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_11(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_10(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_12(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_11(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_13(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_12(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_14(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_13(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_15(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_14(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_16(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_15(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_17(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_16(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_18(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_17(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_19(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_18(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_20(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_19(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_21(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_20(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_22(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_21(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_23(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_22(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_24(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_23(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_25(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_24(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_26(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_25(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_27(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_26(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_28(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_27(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_29(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_28(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_30(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_29(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_31(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_30(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_32(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_31(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_33(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_32(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_34(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_33(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_35(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_34(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_36(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_35(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_37(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_36(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_38(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_37(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_39(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_38(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_40(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_39(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_41(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_40(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_42(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_41(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_43(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_42(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_44(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_43(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_45(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_44(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_46(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_45(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_47(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_46(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_48(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_47(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_49(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_48(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_50(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_49(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_51(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_50(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_52(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_51(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_53(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_52(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_54(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_53(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_55(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_54(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_56(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_55(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_57(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_56(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_58(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_57(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_59(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_58(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_60(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_59(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_61(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_60(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_62(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_61(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_63(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_62(SEQ_REM_HEAD(seq))), () ))
//#define SEQ_TO_LIST_64(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), \
//    (SEQ_GET_HEAD(seq), SEQ_TO_LIST_63(SEQ_REM_HEAD(seq))), () ))


#define SEQ_TO_LIST_1 (head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head) , () ))
#define SEQ_TO_LIST_2 (head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_1 (SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_3 (head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_2 (SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_4 (head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_3 (SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_5 (head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_4 (SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_6 (head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_5 (SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_7 (head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_6 (SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_8 (head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_7 (SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_9 (head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_8 (SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_10(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_9 (SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_11(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_10(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_12(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_11(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_13(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_12(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_14(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_13(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_15(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_14(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_16(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_15(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_17(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_16(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_18(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_17(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_19(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_18(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_20(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_19(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_21(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_20(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_22(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_21(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_23(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_22(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_24(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_23(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_25(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_24(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_26(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_25(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_27(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_26(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_28(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_27(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_29(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_28(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_30(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_29(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_31(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_30(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_32(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_31(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_33(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_32(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_34(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_33(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_35(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_34(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_36(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_35(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_37(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_36(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_38(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_37(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_39(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_38(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_40(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_39(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_41(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_40(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_42(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_41(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_43(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_42(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_44(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_43(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_45(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_44(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_46(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_45(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_47(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_46(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_48(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_47(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_49(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_48(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_50(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_49(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_51(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_50(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_52(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_51(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_53(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_52(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_54(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_53(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_55(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_54(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_56(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_55(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_57(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_56(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_58(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_57(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_59(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_58(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_60(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_59(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_61(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_60(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_62(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_61(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_63(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_62(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
// no comma on first
#define SEQ_TO_LIST_64(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (  head SEQ_TO_LIST_63(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))

#define COMMA_APPEND_IF_NOT_EMPTY(val) IF(IS_NOT_EMPTY(val), (, val), )

#define SEQ_TO_LIST(seq) SEQ_TO_LIST_64(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))

#define SEQ_SIZE(seq) VARIADIC_SIZE(SEQ_TO_LIST(seq))

#define DETAIL_PROTO_REFL_FIELD_TYPE(type, name, meta) type 
#define PROTO_REFL_FIELD_TYPE(...) DETAIL_PROTO_REFL_FIELD_TYPE(__VA_ARGS__)

#define DETAIL_PROTO_REFL_FIELD_NAME(type, name, meta) name 
#define PROTO_REFL_FIELD_NAME(...) DETAIL_PROTO_REFL_FIELD_NAME(__VA_ARGS__)

#define DETAIL_PROTO_REFL_FIELD_META(type, name, meta) meta 
#define PROTO_REFL_FIELD_META(...) DETAIL_PROTO_REFL_FIELD_META(__VA_ARGS__)

#define AT_0(x, ...) x
#define AT_1(x, ...) AT_0(__VA_ARGS__)
#define AT_2(x, ...) AT_1(__VA_ARGS__)
#define AT_3(x, ...) AT_2(__VA_ARGS__)
#define AT_4(x, ...) AT_3(__VA_ARGS__)
#define AT_5(x, ...) AT_4(__VA_ARGS__)
#define AT_6(x, ...) AT_5(__VA_ARGS__)
#define AT_7(x, ...) AT_6(__VA_ARGS__)
#define AT_8(x, ...) AT_7(__VA_ARGS__)
#define AT_9(x, ...) AT_8(__VA_ARGS__)
#define AT_10(x, ...) AT_9(__VA_ARGS__)
#define AT_11(x, ...) AT_10(__VA_ARGS__)
#define AT_12(x, ...) AT_11(__VA_ARGS__)
#define AT_13(x, ...) AT_12(__VA_ARGS__)
#define AT_14(x, ...) AT_13(__VA_ARGS__)
#define AT_15(x, ...) AT_14(__VA_ARGS__)
#define AT_16(x, ...) AT_15(__VA_ARGS__)
#define AT_17(x, ...) AT_16(__VA_ARGS__)
#define AT_18(x, ...) AT_17(__VA_ARGS__)
#define AT_19(x, ...) AT_18(__VA_ARGS__)
#define AT_20(x, ...) AT_19(__VA_ARGS__)
#define AT_21(x, ...) AT_20(__VA_ARGS__)
#define AT_22(x, ...) AT_21(__VA_ARGS__)
#define AT_23(x, ...) AT_22(__VA_ARGS__)
#define AT_24(x, ...) AT_23(__VA_ARGS__)
#define AT_25(x, ...) AT_24(__VA_ARGS__)
#define AT_26(x, ...) AT_25(__VA_ARGS__)
#define AT_27(x, ...) AT_26(__VA_ARGS__)
#define AT_28(x, ...) AT_27(__VA_ARGS__)
#define AT_29(x, ...) AT_28(__VA_ARGS__)
#define AT_30(x, ...) AT_29(__VA_ARGS__)
#define AT_31(x, ...) AT_30(__VA_ARGS__)
#define AT_32(x, ...) AT_31(__VA_ARGS__)
#define AT_33(x, ...) AT_32(__VA_ARGS__)
#define AT_34(x, ...) AT_33(__VA_ARGS__)
#define AT_35(x, ...) AT_34(__VA_ARGS__)
#define AT_36(x, ...) AT_35(__VA_ARGS__)
#define AT_37(x, ...) AT_36(__VA_ARGS__)
#define AT_38(x, ...) AT_37(__VA_ARGS__)
#define AT_39(x, ...) AT_38(__VA_ARGS__)
#define AT_40(x, ...) AT_39(__VA_ARGS__)
#define AT_41(x, ...) AT_40(__VA_ARGS__)
#define AT_42(x, ...) AT_41(__VA_ARGS__)
#define AT_43(x, ...) AT_42(__VA_ARGS__)
#define AT_44(x, ...) AT_43(__VA_ARGS__)
#define AT_45(x, ...) AT_44(__VA_ARGS__)
#define AT_46(x, ...) AT_45(__VA_ARGS__)
#define AT_47(x, ...) AT_46(__VA_ARGS__)
#define AT_48(x, ...) AT_47(__VA_ARGS__)
#define AT_49(x, ...) AT_48(__VA_ARGS__)
#define AT_50(x, ...) AT_49(__VA_ARGS__)
#define AT_51(x, ...) AT_50(__VA_ARGS__)
#define AT_52(x, ...) AT_51(__VA_ARGS__)
#define AT_53(x, ...) AT_52(__VA_ARGS__)
#define AT_54(x, ...) AT_53(__VA_ARGS__)
#define AT_55(x, ...) AT_54(__VA_ARGS__)
#define AT_56(x, ...) AT_55(__VA_ARGS__)
#define AT_57(x, ...) AT_56(__VA_ARGS__)
#define AT_58(x, ...) AT_57(__VA_ARGS__)
#define AT_59(x, ...) AT_58(__VA_ARGS__)
#define AT_60(x, ...) AT_59(__VA_ARGS__)
#define AT_61(x, ...) AT_60(__VA_ARGS__)
#define AT_62(x, ...) AT_61(__VA_ARGS__)
#define AT_63(x, ...) AT_62(__VA_ARGS__)
#define AT_64(x, ...) AT_63(__VA_ARGS__)

#define AT(n, ...) CONCAT(AT_, n)(__VA_ARGS__)

//#define SEQ_AT(n, seq) AT(n, PROTO_SEQ_TO_LIST())
