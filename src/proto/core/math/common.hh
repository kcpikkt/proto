#pragma once
namespace proto {
    float lerp(float a, float b, float t) {
        return a + t * (b-a);
    }
} // namespace proto
