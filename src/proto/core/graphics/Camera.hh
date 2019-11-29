#proto once
#include "proto/core/common/types.hh"
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
    proto::vec3 position;
    float vfov = 45.0f;
    float aspect_ratio = 4.0f/3.0f;
    float near = 0.1f;
    float far  = 1000.0f;

    mat4 projection_matrix() {
        return glm::perspective(vfov, aspect_ratio, near, far);
    } 

    mat4 view_matrix() {
        return glm::translate(proto::mat4(1.0), -position);
    }
}
