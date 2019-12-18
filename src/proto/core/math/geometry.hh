#pragma once
#include "proto/core/common/types.hh"

namespace proto {
    mat4 perspective(float fov, float aspect, float near, float far);
    mat4 translate(mat4 mat,vec3 offset);
    quat angle_axis(float angle,vec3 axis);
} // namespace proto
