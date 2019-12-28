#pragma once
#include "proto/core/preprocessor.hh"
#include "proto/core/common/types.hh"
#include "proto/core/meta.hh"

#define REFL_DETAIL_SEQ_TO_FIELD_REFL(type, name) type, name
#define REFL_SEQ_TO_FIELD_REFL(seq) \
    REFL_DETAIL_SEQ_TO_FIELD_REFL(STR(REM_PAREN(SEQ_AT_0(seq))), \
                                  STR(AT_0(REM_PAREN(SEQ_AT_1(seq))))  \
                                  )

#define REFL_SEQ_TO_FIELD_METADATA(seq) REM_PAREN(IF(IS_NOT_EMPTY(seq), SEQ_AT_2(seq), () )) 

#define REFL_SEQ_TO_FIELD(seq) { {REFL_SEQ_TO_FIELD_REFL(seq)} , REFL_SEQ_TO_FIELD_METADATA(seq) }

#define REFL_SPAWN_FIELD(desc) \
    [[maybe_unused]] REM_PAREN(SEQ_AT_0(desc)) \
    AT_0(REM_PAREN(SEQ_AT_1(desc))) \
    IF(IS_NOT_EMPTY(AT_1(REM_PAREN(SEQ_AT_1(desc)))), = AT_1(REM_PAREN(SEQ_AT_1(desc))), );

#define REFL_SPAWN_FIELDS(...) FOR_EACH_EXP(REFL_SPAWN_FIELD, __VA_ARGS__);

struct Empty{};
#define DETAIL_REFL_FIELDS(...)                                                \
    REFL_SPAWN_FIELDS(__VA_ARGS__)                                             \
    struct Refl {                                                              \
        struct FieldRefl {                                              \
            const char * type;                                          \
            const char * name;                                          \
        };                                                              \
        struct Field : FieldRefl, FieldReflMetadata  {};                       \
        inline constexpr static Field fields[VARIADIC_SIZE(__VA_ARGS__)] =     \
            { FOR_EACH(REFL_SEQ_TO_FIELD, __VA_ARGS__ ) };                     \
        inline constexpr static int fields_count = VARIADIC_SIZE(__VA_ARGS__); \
    };

#define REFL_FIELDS(ReflectedType, MetadataType)                 \
    using Reflected = ReflectedType;                            \
    using FieldReflMetadata = MetadataType;                         \
    DETAIL_REFL_FIELDS 

//hIF(IS_NOT_EMPTY(MetadataType), MetadataType, struct {} ); 


namespace proto {
} // namespace proto
