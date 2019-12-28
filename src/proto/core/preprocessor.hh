#pragma once

#define VARIADIC_SIZE_I(_1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9,  _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, \
                        _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, \
                        _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, size, ...) size

#define VARIADIC_SIZE(...)                                     \
    VARIADIC_SIZE_I(__VA_ARGS__, \
                    64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,    \
                    48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,    \
                    32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,    \
                    16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1, 0)

#define CONCAT_DETAIL1(A,B) A##B
#define CONCAT_DETAIL0(A,B) CONCAT_DETAIL1(A,B)
#define CONCAT(A,B) CONCAT_DETAIL0(A,B)

#define STR(arg) DETAIL_STR(arg)
#define DETAIL_STR(arg) #arg

// for i in $(seq 64); do
// echo "#define FOR_EACH_$i(func, arg, ...) func(arg); FOR_EACH_$((i-1))(func, __VA_ARGS__)"; done

#define DETAIL_REM_HEAD(head, ...) __VA_ARGS__
#define REM_HEAD(...) DETAIL_REM_HEAD(__VA_ARGS__)

#define FOR_EACH_0(...)
#define FOR_EACH_1(func, arg, ...)  COMMA_PREPEND_IF_NOT_EMPTY(func(arg))
#define FOR_EACH_2(func, arg, ...)  COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_1(func, __VA_ARGS__)
#define FOR_EACH_3(func, arg, ...)  COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_2(func, __VA_ARGS__)
#define FOR_EACH_4(func, arg, ...)  COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_3(func, __VA_ARGS__)
#define FOR_EACH_5(func, arg, ...)  COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_4(func, __VA_ARGS__)
#define FOR_EACH_6(func, arg, ...)  COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_5(func, __VA_ARGS__)
#define FOR_EACH_7(func, arg, ...)  COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_6(func, __VA_ARGS__)
#define FOR_EACH_8(func, arg, ...)  COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_7(func, __VA_ARGS__)
#define FOR_EACH_9(func, arg, ...)  COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_8(func, __VA_ARGS__)
#define FOR_EACH_10(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_9(func, __VA_ARGS__)
#define FOR_EACH_11(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_10(func, __VA_ARGS__)
#define FOR_EACH_12(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_11(func, __VA_ARGS__)
#define FOR_EACH_13(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_12(func, __VA_ARGS__)
#define FOR_EACH_14(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_13(func, __VA_ARGS__)
#define FOR_EACH_15(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_14(func, __VA_ARGS__)
#define FOR_EACH_16(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_15(func, __VA_ARGS__)
#define FOR_EACH_17(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_16(func, __VA_ARGS__)
#define FOR_EACH_18(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_17(func, __VA_ARGS__)
#define FOR_EACH_19(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_18(func, __VA_ARGS__)
#define FOR_EACH_20(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_19(func, __VA_ARGS__)
#define FOR_EACH_21(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_20(func, __VA_ARGS__)
#define FOR_EACH_22(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_21(func, __VA_ARGS__)
#define FOR_EACH_23(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_22(func, __VA_ARGS__)
#define FOR_EACH_24(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_23(func, __VA_ARGS__)
#define FOR_EACH_25(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_24(func, __VA_ARGS__)
#define FOR_EACH_26(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_25(func, __VA_ARGS__)
#define FOR_EACH_27(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_26(func, __VA_ARGS__)
#define FOR_EACH_28(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_27(func, __VA_ARGS__)
#define FOR_EACH_29(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_28(func, __VA_ARGS__)
#define FOR_EACH_30(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_29(func, __VA_ARGS__)
#define FOR_EACH_31(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_30(func, __VA_ARGS__)
#define FOR_EACH_32(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_31(func, __VA_ARGS__)
#define FOR_EACH_33(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_32(func, __VA_ARGS__)
#define FOR_EACH_34(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_33(func, __VA_ARGS__)
#define FOR_EACH_35(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_34(func, __VA_ARGS__)
#define FOR_EACH_36(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_35(func, __VA_ARGS__)
#define FOR_EACH_37(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_36(func, __VA_ARGS__)
#define FOR_EACH_38(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_37(func, __VA_ARGS__)
#define FOR_EACH_39(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_38(func, __VA_ARGS__)
#define FOR_EACH_40(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_39(func, __VA_ARGS__)
#define FOR_EACH_41(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_40(func, __VA_ARGS__)
#define FOR_EACH_42(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_41(func, __VA_ARGS__)
#define FOR_EACH_43(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_42(func, __VA_ARGS__)
#define FOR_EACH_44(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_43(func, __VA_ARGS__)
#define FOR_EACH_45(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_44(func, __VA_ARGS__)
#define FOR_EACH_46(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_45(func, __VA_ARGS__)
#define FOR_EACH_47(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_46(func, __VA_ARGS__)
#define FOR_EACH_48(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_47(func, __VA_ARGS__)
#define FOR_EACH_49(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_48(func, __VA_ARGS__)
#define FOR_EACH_50(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_49(func, __VA_ARGS__)
#define FOR_EACH_51(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_50(func, __VA_ARGS__)
#define FOR_EACH_52(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_51(func, __VA_ARGS__)
#define FOR_EACH_53(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_52(func, __VA_ARGS__)
#define FOR_EACH_54(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_53(func, __VA_ARGS__)
#define FOR_EACH_55(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_54(func, __VA_ARGS__)
#define FOR_EACH_56(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_55(func, __VA_ARGS__)
#define FOR_EACH_57(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_56(func, __VA_ARGS__)
#define FOR_EACH_58(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_57(func, __VA_ARGS__)
#define FOR_EACH_59(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_58(func, __VA_ARGS__)
#define FOR_EACH_60(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_59(func, __VA_ARGS__)
#define FOR_EACH_61(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_60(func, __VA_ARGS__)
#define FOR_EACH_62(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_61(func, __VA_ARGS__)
#define FOR_EACH_63(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_62(func, __VA_ARGS__)
#define FOR_EACH_64(func, arg, ...) COMMA_PREPEND_IF_NOT_EMPTY(func(arg)) FOR_EACH_63(func, __VA_ARGS__)
#define FOR_EACH(func, ...) \
    REM_HEAD(CONCAT(FOR_EACH_, VARIADIC_SIZE(__VA_ARGS__))(func, __VA_ARGS__))

#define FOR_EACH_EXP_0(...)
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

#define REM_PAREN(...) IDENTITY __VA_ARGS__

#define DETAIL_SEQ_REM_HEAD_BASE(head, ...) __VA_ARGS__
#define DETAIL_SEQ_REM_HEAD(...) DETAIL_SEQ_REM_HEAD_BASE(__VA_ARGS__)
#define SEQ_REM_HEAD(seq) DETAIL_SEQ_REM_HEAD(SEQ_HEAD_PROBE seq)

#define DETAIL_SEQ_GET_HEAD_BASE(head, ...) head
#define DETAIL_SEQ_GET_HEAD(...) DETAIL_SEQ_GET_HEAD_BASE(__VA_ARGS__)
#define SEQ_GET_HEAD(seq) DETAIL_SEQ_GET_HEAD( IF(IS_NOT_EMPTY(seq), SEQ_HEAD_PROBE,) seq )

// from https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/
#define _ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define HAS_COMMA(...) _ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define _TRIGGER_PARENTHESIS_(...) ,
 
#define IS_EMPTY(...)                                                    \
_IS_EMPTY(                                                               \
          /* test if there is just one argument, eventually an empty    \
             one */                                                     \
          HAS_COMMA(__VA_ARGS__),                                       \
          /* test if _TRIGGER_PARENTHESIS_ together with the argument   \
             adds a comma */                                            \
          HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__),                 \
          /* test if the argument together with a parenthesis           \
             adds a comma */                                            \
          HAS_COMMA(__VA_ARGS__ (/*empty*/)),                           \
          /* test if placing it between _TRIGGER_PARENTHESIS_ and the   \
             parenthesis adds a comma */                                \
          HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/))      \
          )
 
#define PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define _IS_EMPTY(_0, _1, _2, _3) HAS_COMMA(PASTE5(_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define _IS_EMPTY_CASE_0001 ,

#define IS_NOT_EMPTY(...) NOT(IS_EMPTY(__VA_ARGS__))

#define DETAIL_NOT_1() 0
#define DETAIL_NOT_0() 1
#define NOT(VAL) CONCAT(DETAIL_NOT_, VAL)()

#define DETAIL_IF_1(T, F) T
#define DETAIL_IF_0(T, F) F
#define IF(VAL, T, F) CONCAT(DETAIL_IF_, VAL)(T, F)

#define COMMA_PREPEND_IF_NOT_EMPTY(val) REM_PAREN(IF(IS_NOT_EMPTY(val), (, val), () ))
#define COMMA_APPEND_IF_NOT_EMPTY(val) REM_PAREN(IF(IS_NOT_EMPTY(val), (val ,), () ))


#define SEQ_TO_LIST_1(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head ), () ))
#define SEQ_TO_LIST_2(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_1(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_3(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_2(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_4(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_3(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_5(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_4(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_6(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_5(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_7(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_6(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_8(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_7(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_9(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_8(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
#define SEQ_TO_LIST_10(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (, head SEQ_TO_LIST_9(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))
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
// no leading comma here
#define SEQ_TO_LIST_64(head, seq) \
    REM_PAREN(IF(IS_NOT_EMPTY(head), (  head SEQ_TO_LIST_63(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))), () ))

#define SEQ_TO_LIST(seq)  SEQ_TO_LIST_64(SEQ_GET_HEAD(seq), SEQ_REM_HEAD(seq))

#define SEQ_SIZE(seq) VARIADIC_SIZE(SEQ_TO_LIST(seq))

#define DETAIL_PROTO_REFL_FIELD_TYPE(type, name, meta) type 
#define PROTO_REFL_FIELD_TYPE(...) DETAIL_PROTO_REFL_FIELD_TYPE(__VA_ARGS__)

#define DETAIL_PROTO_REFL_FIELD_NAME(type, name, meta) name 
#define PROTO_REFL_FIELD_NAME(...) DETAIL_PROTO_REFL_FIELD_NAME(__VA_ARGS__)

#define DETAIL_PROTO_REFL_FIELD_META(type, name, meta) meta 
#define PROTO_REFL_FIELD_META(...) DETAIL_PROTO_REFL_FIELD_META(__VA_ARGS__)

#define DETAIL_AT_0(x, ...) x
#define DETAIL_AT_1(x, ...) DETAIL_AT_0(__VA_ARGS__)
#define DETAIL_AT_2(x, ...) DETAIL_AT_1(__VA_ARGS__)
#define DETAIL_AT_3(x, ...) DETAIL_AT_2(__VA_ARGS__)
#define DETAIL_AT_4(x, ...) DETAIL_AT_3(__VA_ARGS__)
#define DETAIL_AT_5(x, ...) DETAIL_AT_4(__VA_ARGS__)
#define DETAIL_AT_6(x, ...) DETAIL_AT_5(__VA_ARGS__)
#define DETAIL_AT_7(x, ...) DETAIL_AT_6(__VA_ARGS__)
#define DETAIL_AT_8(x, ...) DETAIL_AT_7(__VA_ARGS__)
#define DETAIL_AT_9(x, ...) DETAIL_AT_8(__VA_ARGS__)
#define DETAIL_AT_10(x, ...) DETAIL_AT_9(__VA_ARGS__)
#define DETAIL_AT_11(x, ...) DETAIL_AT_10(__VA_ARGS__)
#define DETAIL_AT_12(x, ...) DETAIL_AT_11(__VA_ARGS__)
#define DETAIL_AT_13(x, ...) DETAIL_AT_12(__VA_ARGS__)
#define DETAIL_AT_14(x, ...) DETAIL_AT_13(__VA_ARGS__)
#define DETAIL_AT_15(x, ...) DETAIL_AT_14(__VA_ARGS__)
#define DETAIL_AT_16(x, ...) DETAIL_AT_15(__VA_ARGS__)
#define DETAIL_AT_17(x, ...) DETAIL_AT_16(__VA_ARGS__)
#define DETAIL_AT_18(x, ...) DETAIL_AT_17(__VA_ARGS__)
#define DETAIL_AT_19(x, ...) DETAIL_AT_18(__VA_ARGS__)
#define DETAIL_AT_20(x, ...) DETAIL_AT_19(__VA_ARGS__)
#define DETAIL_AT_21(x, ...) DETAIL_AT_20(__VA_ARGS__)
#define DETAIL_AT_22(x, ...) DETAIL_AT_21(__VA_ARGS__)
#define DETAIL_AT_23(x, ...) DETAIL_AT_22(__VA_ARGS__)
#define DETAIL_AT_24(x, ...) DETAIL_AT_23(__VA_ARGS__)
#define DETAIL_AT_25(x, ...) DETAIL_AT_24(__VA_ARGS__)
#define DETAIL_AT_26(x, ...) DETAIL_AT_25(__VA_ARGS__)
#define DETAIL_AT_27(x, ...) DETAIL_AT_26(__VA_ARGS__)
#define DETAIL_AT_28(x, ...) DETAIL_AT_27(__VA_ARGS__)
#define DETAIL_AT_29(x, ...) DETAIL_AT_28(__VA_ARGS__)
#define DETAIL_AT_30(x, ...) DETAIL_AT_29(__VA_ARGS__)
#define DETAIL_AT_31(x, ...) DETAIL_AT_30(__VA_ARGS__)
#define DETAIL_AT_32(x, ...) DETAIL_AT_31(__VA_ARGS__)
#define DETAIL_AT_33(x, ...) DETAIL_AT_32(__VA_ARGS__)
#define DETAIL_AT_34(x, ...) DETAIL_AT_33(__VA_ARGS__)
#define DETAIL_AT_35(x, ...) DETAIL_AT_34(__VA_ARGS__)
#define DETAIL_AT_36(x, ...) DETAIL_AT_35(__VA_ARGS__)
#define DETAIL_AT_37(x, ...) DETAIL_AT_36(__VA_ARGS__)
#define DETAIL_AT_38(x, ...) DETAIL_AT_37(__VA_ARGS__)
#define DETAIL_AT_39(x, ...) DETAIL_AT_38(__VA_ARGS__)
#define DETAIL_AT_40(x, ...) DETAIL_AT_39(__VA_ARGS__)
#define DETAIL_AT_41(x, ...) DETAIL_AT_40(__VA_ARGS__)
#define DETAIL_AT_42(x, ...) DETAIL_AT_41(__VA_ARGS__)
#define DETAIL_AT_43(x, ...) DETAIL_AT_42(__VA_ARGS__)
#define DETAIL_AT_44(x, ...) DETAIL_AT_43(__VA_ARGS__)
#define DETAIL_AT_45(x, ...) DETAIL_AT_44(__VA_ARGS__)
#define DETAIL_AT_46(x, ...) DETAIL_AT_45(__VA_ARGS__)
#define DETAIL_AT_47(x, ...) DETAIL_AT_46(__VA_ARGS__)
#define DETAIL_AT_48(x, ...) DETAIL_AT_47(__VA_ARGS__)
#define DETAIL_AT_49(x, ...) DETAIL_AT_48(__VA_ARGS__)
#define DETAIL_AT_50(x, ...) DETAIL_AT_49(__VA_ARGS__)
#define DETAIL_AT_51(x, ...) DETAIL_AT_50(__VA_ARGS__)
#define DETAIL_AT_52(x, ...) DETAIL_AT_51(__VA_ARGS__)
#define DETAIL_AT_53(x, ...) DETAIL_AT_52(__VA_ARGS__)
#define DETAIL_AT_54(x, ...) DETAIL_AT_53(__VA_ARGS__)
#define DETAIL_AT_55(x, ...) DETAIL_AT_54(__VA_ARGS__)
#define DETAIL_AT_56(x, ...) DETAIL_AT_55(__VA_ARGS__)
#define DETAIL_AT_57(x, ...) DETAIL_AT_56(__VA_ARGS__)
#define DETAIL_AT_58(x, ...) DETAIL_AT_57(__VA_ARGS__)
#define DETAIL_AT_59(x, ...) DETAIL_AT_58(__VA_ARGS__)
#define DETAIL_AT_60(x, ...) DETAIL_AT_59(__VA_ARGS__)
#define DETAIL_AT_61(x, ...) DETAIL_AT_60(__VA_ARGS__)
#define DETAIL_AT_62(x, ...) DETAIL_AT_61(__VA_ARGS__)
#define DETAIL_AT_63(x, ...) DETAIL_AT_62(__VA_ARGS__)
#define DETAIL_AT_64(x, ...) DETAIL_AT_63(__VA_ARGS__)

// VA_ARGS have to be laundered
#define AT_0(...)  DETAIL_AT_0(__VA_ARGS__)
#define AT_1(...)  DETAIL_AT_1(__VA_ARGS__)
#define AT_2(...)  DETAIL_AT_2(__VA_ARGS__)
#define AT_3(...)  DETAIL_AT_3(__VA_ARGS__)
#define AT_4(...)  DETAIL_AT_4(__VA_ARGS__)
#define AT_5(...)  DETAIL_AT_5(__VA_ARGS__)
#define AT_6(...)  DETAIL_AT_6(__VA_ARGS__)
#define AT_7(...)  DETAIL_AT_7(__VA_ARGS__)
#define AT_8(...)  DETAIL_AT_8(__VA_ARGS__)
#define AT_9(...)  DETAIL_AT_9(__VA_ARGS__)
#define AT_10(...) DETAIL_AT_10(__VA_ARGS__)
#define AT_11(...) DETAIL_AT_11(__VA_ARGS__)
#define AT_12(...) DETAIL_AT_12(__VA_ARGS__)
#define AT_13(...) DETAIL_AT_13(__VA_ARGS__)
#define AT_14(...) DETAIL_AT_14(__VA_ARGS__)
#define AT_15(...) DETAIL_AT_15(__VA_ARGS__)
#define AT_16(...) DETAIL_AT_16(__VA_ARGS__)
#define AT_17(...) DETAIL_AT_17(__VA_ARGS__)
#define AT_18(...) DETAIL_AT_18(__VA_ARGS__)
#define AT_19(...) DETAIL_AT_19(__VA_ARGS__)
#define AT_20(...) DETAIL_AT_20(__VA_ARGS__)
#define AT_21(...) DETAIL_AT_21(__VA_ARGS__)
#define AT_22(...) DETAIL_AT_22(__VA_ARGS__)
#define AT_23(...) DETAIL_AT_23(__VA_ARGS__)
#define AT_24(...) DETAIL_AT_24(__VA_ARGS__)
#define AT_25(...) DETAIL_AT_25(__VA_ARGS__)
#define AT_26(...) DETAIL_AT_26(__VA_ARGS__)
#define AT_27(...) DETAIL_AT_27(__VA_ARGS__)
#define AT_28(...) DETAIL_AT_28(__VA_ARGS__)
#define AT_29(...) DETAIL_AT_29(__VA_ARGS__)
#define AT_30(...) DETAIL_AT_30(__VA_ARGS__)
#define AT_31(...) DETAIL_AT_31(__VA_ARGS__)
#define AT_32(...) DETAIL_AT_32(__VA_ARGS__)
#define AT_33(...) DETAIL_AT_33(__VA_ARGS__)
#define AT_34(...) DETAIL_AT_34(__VA_ARGS__)
#define AT_35(...) DETAIL_AT_35(__VA_ARGS__)
#define AT_36(...) DETAIL_AT_36(__VA_ARGS__)
#define AT_37(...) DETAIL_AT_37(__VA_ARGS__)
#define AT_38(...) DETAIL_AT_38(__VA_ARGS__)
#define AT_39(...) DETAIL_AT_39(__VA_ARGS__)
#define AT_40(...) DETAIL_AT_40(__VA_ARGS__)
#define AT_41(...) DETAIL_AT_41(__VA_ARGS__)
#define AT_42(...) DETAIL_AT_42(__VA_ARGS__)
#define AT_43(...) DETAIL_AT_43(__VA_ARGS__)
#define AT_44(...) DETAIL_AT_44(__VA_ARGS__)
#define AT_45(...) DETAIL_AT_45(__VA_ARGS__)
#define AT_46(...) DETAIL_AT_46(__VA_ARGS__)
#define AT_47(...) DETAIL_AT_47(__VA_ARGS__)
#define AT_48(...) DETAIL_AT_48(__VA_ARGS__)
#define AT_49(...) DETAIL_AT_49(__VA_ARGS__)
#define AT_50(...) DETAIL_AT_50(__VA_ARGS__)
#define AT_51(...) DETAIL_AT_51(__VA_ARGS__)
#define AT_52(...) DETAIL_AT_52(__VA_ARGS__)
#define AT_53(...) DETAIL_AT_53(__VA_ARGS__)
#define AT_54(...) DETAIL_AT_54(__VA_ARGS__)
#define AT_55(...) DETAIL_AT_55(__VA_ARGS__)
#define AT_56(...) DETAIL_AT_56(__VA_ARGS__)
#define AT_57(...) DETAIL_AT_57(__VA_ARGS__)
#define AT_58(...) DETAIL_AT_58(__VA_ARGS__)
#define AT_59(...) DETAIL_AT_59(__VA_ARGS__)
#define AT_60(...) DETAIL_AT_60(__VA_ARGS__)
#define AT_61(...) DETAIL_AT_61(__VA_ARGS__)
#define AT_62(...) DETAIL_AT_62(__VA_ARGS__)
#define AT_63(...) DETAIL_AT_63(__VA_ARGS__)
#define AT_64(...) DETAIL_AT_64(__VA_ARGS__)

#define DETAIL_AT(n, ...) CONCAT(DETAIL_AT_, n)(__VA_ARGS__)
#define AT(...) IDENTITY(DETAIL_AT(__VA_ARGS__))
// VA_ARGS have to be laundered

#define SEQ_AT_0(sqn)   AT_0(SEQ_TO_LIST(sqn))
#define SEQ_AT_1(sqn)   AT_1(SEQ_TO_LIST(sqn))
#define SEQ_AT_2(sqn)   AT_2(SEQ_TO_LIST(sqn))
#define SEQ_AT_3(sqn)   AT_3(SEQ_TO_LIST(sqn))
#define SEQ_AT_4(sqn)   AT_4(SEQ_TO_LIST(sqn))
#define SEQ_AT_5(sqn)   AT_5(SEQ_TO_LIST(sqn))
#define SEQ_AT_6(sqn)   AT_6(SEQ_TO_LIST(sqn))
#define SEQ_AT_7(sqn)   AT_7(SEQ_TO_LIST(sqn))
#define SEQ_AT_8(sqn)   AT_8(SEQ_TO_LIST(sqn))
#define SEQ_AT_9(sqn)   AT_9(SEQ_TO_LIST(sqn))
#define SEQ_AT_10(sqn) AT_10(SEQ_TO_LIST(sqn))
#define SEQ_AT_11(sqn) AT_11(SEQ_TO_LIST(sqn))
#define SEQ_AT_12(sqn) AT_12(SEQ_TO_LIST(sqn))
#define SEQ_AT_13(sqn) AT_13(SEQ_TO_LIST(sqn))
#define SEQ_AT_14(sqn) AT_14(SEQ_TO_LIST(sqn))
#define SEQ_AT_15(sqn) AT_15(SEQ_TO_LIST(sqn))
#define SEQ_AT_16(sqn) AT_16(SEQ_TO_LIST(sqn))
#define SEQ_AT_17(sqn) AT_17(SEQ_TO_LIST(sqn))
#define SEQ_AT_18(sqn) AT_18(SEQ_TO_LIST(sqn))
#define SEQ_AT_19(sqn) AT_19(SEQ_TO_LIST(sqn))
#define SEQ_AT_20(sqn) AT_20(SEQ_TO_LIST(sqn))
#define SEQ_AT_21(sqn) AT_21(SEQ_TO_LIST(sqn))
#define SEQ_AT_22(sqn) AT_22(SEQ_TO_LIST(sqn))
#define SEQ_AT_23(sqn) AT_23(SEQ_TO_LIST(sqn))
#define SEQ_AT_24(sqn) AT_24(SEQ_TO_LIST(sqn))
#define SEQ_AT_25(sqn) AT_25(SEQ_TO_LIST(sqn))
#define SEQ_AT_26(sqn) AT_26(SEQ_TO_LIST(sqn))
#define SEQ_AT_27(sqn) AT_27(SEQ_TO_LIST(sqn))
#define SEQ_AT_28(sqn) AT_28(SEQ_TO_LIST(sqn))
#define SEQ_AT_29(sqn) AT_29(SEQ_TO_LIST(sqn))
#define SEQ_AT_30(sqn) AT_30(SEQ_TO_LIST(sqn))
#define SEQ_AT_31(sqn) AT_31(SEQ_TO_LIST(sqn))
#define SEQ_AT_32(sqn) AT_32(SEQ_TO_LIST(sqn))
#define SEQ_AT_33(sqn) AT_33(SEQ_TO_LIST(sqn))
#define SEQ_AT_34(sqn) AT_34(SEQ_TO_LIST(sqn))
#define SEQ_AT_35(sqn) AT_35(SEQ_TO_LIST(sqn))
#define SEQ_AT_36(sqn) AT_36(SEQ_TO_LIST(sqn))
#define SEQ_AT_37(sqn) AT_37(SEQ_TO_LIST(sqn))
#define SEQ_AT_38(sqn) AT_38(SEQ_TO_LIST(sqn))
#define SEQ_AT_39(sqn) AT_39(SEQ_TO_LIST(sqn))
#define SEQ_AT_40(sqn) AT_40(SEQ_TO_LIST(sqn))
#define SEQ_AT_41(sqn) AT_41(SEQ_TO_LIST(sqn))
#define SEQ_AT_42(sqn) AT_42(SEQ_TO_LIST(sqn))
#define SEQ_AT_43(sqn) AT_43(SEQ_TO_LIST(sqn))
#define SEQ_AT_44(sqn) AT_44(SEQ_TO_LIST(sqn))
#define SEQ_AT_45(sqn) AT_45(SEQ_TO_LIST(sqn))
#define SEQ_AT_46(sqn) AT_46(SEQ_TO_LIST(sqn))
#define SEQ_AT_47(sqn) AT_47(SEQ_TO_LIST(sqn))
#define SEQ_AT_48(sqn) AT_48(SEQ_TO_LIST(sqn))
#define SEQ_AT_49(sqn) AT_49(SEQ_TO_LIST(sqn))
#define SEQ_AT_50(sqn) AT_50(SEQ_TO_LIST(sqn))
#define SEQ_AT_51(sqn) AT_51(SEQ_TO_LIST(sqn))
#define SEQ_AT_52(sqn) AT_52(SEQ_TO_LIST(sqn))
#define SEQ_AT_53(sqn) AT_53(SEQ_TO_LIST(sqn))
#define SEQ_AT_54(sqn) AT_54(SEQ_TO_LIST(sqn))
#define SEQ_AT_55(sqn) AT_55(SEQ_TO_LIST(sqn))
#define SEQ_AT_56(sqn) AT_56(SEQ_TO_LIST(sqn))
#define SEQ_AT_57(sqn) AT_57(SEQ_TO_LIST(sqn))
#define SEQ_AT_58(sqn) AT_58(SEQ_TO_LIST(sqn))
#define SEQ_AT_59(sqn) AT_59(SEQ_TO_LIST(sqn))
#define SEQ_AT_60(sqn) AT_60(SEQ_TO_LIST(sqn))
#define SEQ_AT_61(sqn) AT_61(SEQ_TO_LIST(sqn))
#define SEQ_AT_62(sqn) AT_62(SEQ_TO_LIST(sqn))
#define SEQ_AT_63(sqn) AT_63(SEQ_TO_LIST(sqn))
#define SEQ_AT_64(sqn) AT_64(SEQ_TO_LIST(sqn))

#define SEQ_AT(n, sqn) CONCAT(SEQ_AT_, n)(sqn)



//#define SEQ_AT(n, seq) AT(n, PROTO_SEQ_TO_LIST())
