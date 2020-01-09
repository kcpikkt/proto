#pragma once
#include "proto/core/common/types.hh"
namespace proto {
    struct Vertex {
        vec3 position;
        vec3 normal;
        vec3 tangent;
        vec2 uv;
    };
} // namespace proto
