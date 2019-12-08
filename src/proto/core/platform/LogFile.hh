#pragma once
namespace proto {

struct LogFile {
    Bitfield<debug::Category> stdout_log_categories = debug::category::all;
    debug::Level stdout_log_level = debug::level::all;
}

} // namespace proto
