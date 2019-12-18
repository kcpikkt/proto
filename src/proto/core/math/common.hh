#pragma once
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace proto {
    template<typename T>
    T lerp(T a, T b, float t) {
        return a + t * (b-a);
    }
} // namespace proto
