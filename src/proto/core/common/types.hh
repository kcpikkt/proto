#pragma once

#include <stdint.h>
#include <stddef.h>
#define GLM_FORCE_SWIZZLE 
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
    // never used SIMD, dont feel like writing my
    // own math lib now but perhaps in future?

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

    using quat = glm::quat;

    template<typename T = u64> struct Span {
        T begin = 0;
        T size = 0;

        Span() {}
        Span(T _begin, T _size) : begin(_begin), size(_size) {}
    };

    template<typename T = u64>
    struct Range {
        T begin = 0;
        T end = 0;

        Range() {};
        Range(T _begin, T _end) : begin(_begin), end(_end) {}
    };

    template<typename T>
    struct Buffer {
        union {
            T * data = nullptr;
            u8   * data8;
            u16  * data16;
            u32  * data32;
            u64  * data64;
        };
        size_t size = 0;

        operator bool() {
            return (bool)data;
        }
    };

    using MemBuffer = Buffer<void>;
    using StrBuffer = Buffer<char>;
} // namespace proto
