#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/util/Bitfield.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/debug/common.hh"

namespace proto {

struct RuntimeSettings {
    constexpr static u8 terminal_mode_bit = BIT(0);
    constexpr static u8 graphics_mode_bit = BIT(1);

    Bitfield<u8> mode = terminal_mode_bit | graphics_mode_bit;

    Bitfield<debug::Category> init_stdout_log_categories = debug::category::all;
    debug::Level init_stdout_log_level = debug::level::all;

    StringView asset_paths;
};

} // namespace proto
