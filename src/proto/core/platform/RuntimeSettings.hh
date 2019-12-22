#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/util/Bitfield.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/debug/common.hh"

namespace proto {

// remember to copy, don't use after setup
struct RuntimeSettings {
    constexpr static u8 terminal_mode_bit = BIT(0);
    constexpr static u8 window_mode_bit = BIT(1);
    constexpr static u8 opengl_mode_bit = BIT(2);
    Bitfield<u8> mode =
        terminal_mode_bit |
        window_mode_bit |
        opengl_mode_bit;

    Bitfield<debug::Category> init_stdout_log_categories = debug::category::all;
    debug::Level init_stdout_log_level = debug::level::all;

    StringView asset_paths;
    StringView shader_paths;

    // this e.g. is super temporary
    //    StringView * cmdline_sentences = nullptr;
    //    s32 cmdline_sentences_count;
    //    // NOTE(kacper): whould be cool to have my Array here but how to make
    //    //               braced list ctor for custom type without stl?
    //    //               perhaps get second type of Array that does not allocate,
    //    //               that can be cast-to from carray?

    u64 mt64_seed = 5429;
};

} // namespace proto
