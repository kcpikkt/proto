#pragma once
#include "proto/core/common.hh"
#include <chrono>
#include <iostream>

namespace proto {

    //NOTE(kacper): friendship is not inherited
    class Application;

    class Time {
    public:
        using time_t         = float;
        using duration_t     = std::chrono::duration<time_t, std::ratio<1>>;
        using clock_t        = std::chrono::high_resolution_clock;
        using time_point_t   = std::chrono::time_point<clock_t>;

        inline duration_t time() {
            return _time;
        }

        inline duration_t deltatime() {
            return _deltatime;
        }

        inline time_t time_c() {
            return _time.count();
        }

        inline time_t deltatime_c() {
            return _deltatime.count();
        }

        static inline time_point_t now() {
            return clock_t::now();
        }
    private:
        constexpr static duration_t zero_duration = duration_t((time_t)0);

        time_point_t _init_time_point;
        time_point_t _last_tick_time_point;

        duration_t _time;
        duration_t _deltatime;
        duration_t _last_deltatime;

        void init() {
            _init_time_point = now();
            _last_tick_time_point = _init_time_point;

            _last_deltatime = _deltatime = _time = zero_duration;
        }
        void tick() {
            _last_deltatime = _deltatime;
            _deltatime = std::chrono::duration_cast<duration_t>
                (now() - _last_tick_time_point);

            _time += _deltatime;

            _last_tick_time_point = now();
        }

        friend Application;
    };
}
