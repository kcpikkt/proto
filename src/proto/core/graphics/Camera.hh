#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/math/geometry.hh"
#include "proto/core/math/common.hh"

namespace proto {
    struct Camera {
        vec3 position;
        quat rotation;

        float fov = 45.0f;
        float aspect = 4.0f/3.0f;
        float near = 0.1f;
        float far  = 100.0f;
        
        Camera() {}
        Camera(float fov, float aspect, float far, float near)
            : fov(fov), aspect(aspect), near(near), far(far)
        {}

        mat4 projection() {
            return glm::perspective(fov, aspect, near, far);
        } 

        mat4 view() {
            mat4 ret = mat4(1.0);
            ret = glm::toMat4(glm::conjugate(rotation)) * ret;
            return glm::translate(ret, -position);
        }

        mat4 projection_matrix() {
            PROTO_DEPRECATED;
            return glm::perspective(fov, aspect, near, far);
        } 

    };
} // namespace proto
