#pragma once

namespace proto{
    // todo enable_if is integral unsigned
    template<typename I>
    static inline void bitfield_set(I * bitfield, I flag) {
        *bitfield |= flag;
    }

    template<typename I>
    static inline void bitfield_unset(I * bitfield, I flag) {
        *bitfield &= ~flag;
    }

    template<typename I>
    static inline void bitfield_toggle(I * bitfield, I flag) {
        *bitfield ^= flag;
    }
} // namespace proto
