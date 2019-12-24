#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/platform/macros.hh"

namespace proto {
namespace platform {

struct Clock {
    float delta_time;
    float elapsed_time;
    u64 tick_count = 0;

#if defined(PROTO_PLATFORM_LINUX)
    using nsec_t = decltype(timespec::tv_nsec);

    constexpr static nsec_t one_sec = 1000000000;

    nsec_t _delta_time_variance_nsec = 0;
    nsec_t _delta_time_standard_deviation_nsec = 0;
    nsec_t _delta_time_mean_nsec = 0;
    //private:
    nsec_t _delta_time_variance_sum = 0;
    nsec_t _delta_time_variance_sqsum = 0;
    nsec_t _delta_time_variance_n = 0;
    //public:

    struct timespec _init_time = {};
    struct timespec _elapsed_time = {};
    struct timespec _frame_start_time = {};
    struct timespec _prev_frame_start_time = {};
    struct timespec _work_end_time  = {};
    struct timespec _frame_end_time = {};

    struct timespec _work_time  = {};
    struct timespec _sleep_time = {};
    struct timespec _delta_time = {};

    struct timespec _desired_time;
#elif defined(PROTO_PLATFORM_WINDOWS)
    #error not implemented
#endif

#if defined(PROTO_PLATFORM_LINUX)
    void timespec_reduce(timespec* ts);
#endif
    void init(float fps);
    void tick();
};

} // namespace platform
} // namespace proto
