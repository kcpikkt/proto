#pragma once

#include <cstdint>
#include <cstddef>
#include <glm/glm.hpp>

//TEMP
#include <memory>

namespace proto {
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using s8  = int8_t;
    using s16 = int16_t;
    using s32 = int32_t;
    using s64 = int64_t;

    // NOTE(kacper):
    // this is for forward compatibility when I am going implement
    // math library. glm is used for now since I never used SIMD intrinsics
    // before and i dont feel like writing it now now

    using ivec2 = glm::ivec2;
    using ivec3 = glm::ivec3;
    using ivec4 = glm::ivec4;

    using uvec2 = glm::uvec2;
    using uvec3 = glm::uvec3;
    using uvec4 = glm::uvec4;

    using vec2 = glm::vec2;
    using vec3 = glm::vec3;
    using vec4 = glm::vec4;

    using mat2 = glm::mat2;
    using mat3 = glm::mat3;
    using mat4 = glm::mat4;
    using mat2x2 = glm::mat2x2;
    using mat3x3 = glm::mat3x3;
    using mat4x4 = glm::mat4x4;

}
