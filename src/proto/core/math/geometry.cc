#include "proto/core/math/geometry.hh"

namespace proto {
    mat4 perspective(float fov, float aspect, float near, float far) {
        float theta = tan(fov/2.0);
        // row here is column in matrix btw
        return {{1.0/theta, 0.0,         0.0,                          0.0},
                {0.0,      aspect/theta, 0.0,                          0.0},
                {0.0,      0.0,         -2.0/(near - far),            -1.0},
                {0.0,      0.0,          2.0 * far/(near - far) + 1.0, 0.0}};
    }

} // namespace proto
