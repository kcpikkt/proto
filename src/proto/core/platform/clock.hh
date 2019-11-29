#pragma once
#include "proto/core/common/types.hh"
namespace proto {
namespace platform {

template<typename Impl>
struct ClockCRTPInterface {
    float delta_time;
    float elapsed_time;
    u64 tick_count = 0;
};

} // namespace platform
} // namespace proto
