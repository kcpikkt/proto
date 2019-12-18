#include "proto/core/math/geometry.hh"
#include <tgmath.h>

namespace proto {
    mat4 perspective(float fov, float aspect, float near, float far) {
    // FIXME(kacper): non-linearity of depth mate, this function does not work
        assert(0);
        float theta = tan(fov/2.0);
        // row here is column in matrix btw
        return {{1.0/theta, 0.0,         0.0,                          0.0},
                {0.0,      aspect/theta, 0.0,                          0.0},
                {0.0,      0.0,         -2.0/(near - far),            -1.0},
                {0.0,      0.0,          2.0 * far/(near - far) + 1.0, 0.0}};
    }

    mat4 translate(mat4 mat, vec3 offset) {
        return mat4(mat[0], mat[1], mat[2], vec4(mat[3][0] + offset.x,
                                                 mat[3][1] + offset.y,
                                                 mat[3][2] + offset.z,
                                                 mat[3][3]));
    }

    quat angle_axis(float angle,vec3 axis) {
        return quat(float(cos(angle/2.0f)), float(sin(angle/2.0f)) * axis);
    }


} // namespace proto
