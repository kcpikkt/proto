#pragma once
namespace proto {
//TODO(kcaper): declval in SFINAE decltypes! 

template<typename A, typename B,
         typename HigherPrecType = decltype(A() + B()),
         // SFINAE: allow if there exists comparison operator between types
         //         used so there is no need for cast to one type when calling
         typename = decltype(A() < B())> 
inline HigherPrecType max(A a, B b) {
    return ((HigherPrecType)a < (HigherPrecType)b ? b : a);
}

template<typename A, typename B,
         typename HigherPrecType = decltype(A() + B()),
         // SFINAE: same as above
         typename = decltype(A() < B())> 
inline HigherPrecType min(A a, B b) {
    return ((HigherPrecType)a < (HigherPrecType)b ? a : b);
}

template<typename A, typename B = float,
         typename HigherPrecType = decltype(A() + B()),
         // SFINAE: same as above
         typename = decltype(A() < B())> 
inline HigherPrecType clamp(A val, B minval = 0.0f, B maxval = 1.0f) {
    return max(minval, min(maxval, (HigherPrecType)val));
}

constexpr inline size_t next_multiple(size_t of, size_t val) {
    return val + (of - (val % of));
}

template<typename A, typename B,
         // SFINAE: same as above
         typename = decltype(A() <  B()),
         typename = decltype(A() >= B())> 
bool belongs(A value, B lower, B upper) {
    return (value >= lower && value < upper);
}

} // namespace proto
