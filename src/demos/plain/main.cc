#include "proto/proto.hh"

#include "proto/core/reflection.hh" 
#include "proto/core/meta.hh" 
#include "proto/core/util/String.hh" 
#include "proto/core/util/Buffer.hh" 

struct GLSLMaterialFieldRefl {
    const char * glsl_type = nullptr;
    const char * glsl_name = nullptr;
};


#define ARG3 (int) (name, 10) ({}), (int) (ii) ({}), (int) (test) ({}) 
struct Mat {
    REFL_FIELDS(Mat, GLSLMaterialFieldRefl)(
        (int) (a) ({"sampler2D", "a"}),
        (int) (b) ({"sampler2D", "b"})
    );
};
#define TEST(seq) REM_PAREN(SEQ_AT_0(seq)) REM_PAREN(SEQ_AT_1(seq));

#define ARG0 
#define ARG1 (int) (name) ({"sampler2D", "sampler"})
#define ARG2 (int) (name) ({}), (int) (ii) ({})
#define ARG9 1,2,3,4,5,6,7,8,9
//struct Test {};
//struct Mat {
//    using FieldReflMetadata = Test;
//    int name;
//    struct Refl {
//        struct Field : FieldRefl, FieldReflMetadata {};
//        inline constexpr static Field fields[1] = { { { "int", "name" }, {} } };
//        inline constexpr static int fields_count = 1;
//    };
//};

using namespace proto;

u64 glsl_material_struct_def_string(StrBuffer buffer) {
    u64 written = 0;
    written += sprint(buffer.data, buffer.size, "layout(std140) uniform Material {\n");
    for(auto& field : Mat::Refl::fields) {
        written += sprint(buffer.data + written, buffer.size - written,
                          "    ", field.glsl_type, ' ', field.glsl_name, "; \n");
    }
    written += sprint(buffer.data + written, buffer.size - written, "};");
    return written;
}


PROTO_INIT {

    StrBuffer buffer = {{(char*)context->memory.alloc(32)}, 32};
    glsl_material_struct_def_string(buffer);
    vardump(buffer.data);
    //for(auto & field : Mat::Refl::fields) {
    //    printf("%s %s %s %s\n", field.type, field.name, field.glsl_type, field.glsl_name);
    //    Mat a = Mat();


    //}
}
