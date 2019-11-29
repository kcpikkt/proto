#pragma once
#include "proto/core/io.hh"
#include "proto/core/common.hh"
#include "proto/core/meta.hh"
namespace proto {
    struct TerminalProgressBar {
        union {
            float float_target = 100.0f;
            u64 int_target;
        };
        char rbound = '[';
        char fill = '#';
        char empty = '-';
        char lbound = ']';
        u8 segments =  10;
        enum {ProgressPercentage, ProgressRatio}
        progress_type = ProgressPercentage;
    };

    template<typename T>
    void print_terminal_progress_bar(T val, TerminalProgressBar * bar) {
        static_assert(proto::meta::is_integer<T>::value ||
                      proto::meta::is_floating_point<T>::value);

        float progress;
        if constexpr (proto::meta::is_integer<T>::value) {
            assert(bar->int_target != 0);
            progress = clamp((float)val / (float)bar->int_target);
        } else if(proto::meta::is_floating_point<T>::value) {
            assert(bar->float_target != 0);
            progress = clamp(val / bar->float_target);
        } 
        if(bar->rbound) io::print(bar->rbound);

        s32 i = 0;
        for(; i < min((s32)(progress * bar->segments), bar->segments); i++)
            io::print(bar->fill);

        for(; i < bar->segments; i++)
            io::print(bar->empty);

        if(bar->lbound) io::print(bar->lbound);
        io::print(' ');

        if(bar->progress_type == TerminalProgressBar::ProgressPercentage) {
            printf("%.1f%%", 100.0f * progress);
        } else {
            if constexpr (proto::meta::is_integer<T>::value) {
                io::print(val, "/", bar->int_target);
            } else {
                assert(0);
            }
        }
        io::print('\r');
        io::flush();
    }
} // namespace proto


