#include "proto/core/platform/macros.hh"
#if defined(PROTO_PLATFORM_LINUX)

#include "proto/core/platform/linux-clock.hh"

#include <time.h>
#include <stdio.h>
#include <tgmath.h>

void proto::platform::LinuxClockImpl::timespec_reduce(timespec* ts) {
    ts->tv_sec  += ts->tv_nsec / one_sec;
    ts->tv_nsec %= one_sec;
}

void proto::platform::LinuxClockImpl::init(float fps) {
    _desired_time.tv_sec = 0;
    _desired_time.tv_nsec = one_sec / fps;

    timespec_reduce(&_desired_time);

    _delta_time = _desired_time; //HACK?
    clock_gettime(CLOCK_MONOTONIC_RAW, &_frame_start_time);
    _init_time = _frame_start_time;
}

timespec timespec_diff(timespec& fst, timespec& snd) {
    constexpr static decltype(timespec::tv_nsec) one_sec = 1000000000;
    timespec ret;
    ret.tv_sec = fst.tv_sec - snd.tv_sec;
    if(ret.tv_sec == 0) {
        ret.tv_nsec = fst.tv_nsec - snd.tv_nsec;
    } else if(ret.tv_sec > 0) {
        ret.tv_nsec = fst.tv_nsec + (one_sec - snd.tv_nsec);
    } else if(ret.tv_sec < 0) {
        ret.tv_nsec = (fst.tv_nsec - one_sec) - snd.tv_sec;
    }
    return ret;

}

void proto::platform::LinuxClockImpl::tick() {
    tick_count += 1;
    clock_gettime(CLOCK_MONOTONIC_RAW, &_work_end_time);

    _work_time.tv_sec  = _work_end_time.tv_sec - _frame_start_time.tv_sec;
    _work_time.tv_nsec = _work_end_time.tv_nsec - _frame_start_time.tv_nsec;

    _sleep_time.tv_sec = 0;
    _sleep_time.tv_nsec = 0;

    //_SLEEP
    if(_work_time.tv_sec == _desired_time.tv_sec) {
        if(_work_time.tv_nsec < _desired_time.tv_nsec) {
            _sleep_time.tv_nsec =
                _desired_time.tv_nsec -
                _work_time.tv_nsec;
        }
        nanosleep(&_sleep_time, NULL);
    } else if(_work_time.tv_sec < _desired_time.tv_sec) {
        decltype(_sleep_time.tv_nsec) one_nsec = 1000000000;

        _sleep_time.tv_sec =
            _desired_time.tv_sec -
            _work_time.tv_sec - 1;
        _sleep_time.tv_nsec =
            one_nsec - _work_time.tv_nsec +
            _desired_time.tv_nsec ;

        _sleep_time.tv_sec += _sleep_time.tv_nsec / one_nsec;
        _sleep_time.tv_nsec = _sleep_time.tv_nsec % one_nsec;

        nanosleep(&_sleep_time, NULL);
    }


    clock_gettime(CLOCK_MONOTONIC_RAW, &_frame_end_time);

    //-----------------------

    _prev_frame_start_time = _frame_start_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &_frame_start_time);

    _delta_time = timespec_diff(_frame_end_time, _prev_frame_start_time);

    delta_time = (float) _delta_time.tv_nsec / one_sec;

    _elapsed_time = timespec_diff(_frame_end_time, _init_time);

    elapsed_time =
        (float) _elapsed_time.tv_sec +
        (float) _elapsed_time.tv_nsec / one_sec;

    long _delta_time_variance_sample = _desired_time.tv_nsec - _delta_time.tv_nsec;

    _delta_time_variance_sum += _delta_time_variance_sample;
    _delta_time_variance_sqsum +=
        _delta_time_variance_sample * _delta_time_variance_sample;
    _delta_time_variance_nsec = (_delta_time_variance_sqsum -
                                _delta_time_variance_sum *
                                _delta_time_variance_sum /
                                tick_count);

    _delta_time_variance_nsec /= (tick_count);
    _delta_time_standard_deviation_nsec =
        (long)sqrt(_delta_time_variance_nsec);

    //-----------------------
}

#endif
