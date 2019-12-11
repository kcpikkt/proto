#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/math/geometry.hh"
#include <glm/gtc/matrix_transform.hpp>

namespace proto {
    struct Camera {
        vec3 position;
        vec3 orientation;

        float fov = 45.0f;
        float aspect = 4.0f/3.0f;
        float near = 0.1f;
        float far  = 1000.0f;
        
        Camera() {}
        Camera(float fov, float aspect, float far, float near)
            : fov(fov), aspect(aspect), near(near), far(far)
        {}

        mat4 projection_matrix() {
            return perspective(fov, aspect, near, far);
        } 

        mat4 view_matrix() {
            return glm::translate(proto::mat4(1.0), -position);
        }
    };
} // namespace proto
