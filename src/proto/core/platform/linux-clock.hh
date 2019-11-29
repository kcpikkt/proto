#pragma once
#include "proto/core/platform/macros.hh"
#if defined(PROTO_PLATFORM_LINUX)

#include "proto/core/platform/clock.hh"
#include <time.h>

namespace proto {
namespace platform {

struct LinuxClockImpl : ClockCRTPInterface<LinuxClockImpl> {
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

    void timespec_reduce(timespec* ts);
    void init(float fps);
    void tick();
};

}
}

#else
#error linux specific header included in non linux platform
#endif
