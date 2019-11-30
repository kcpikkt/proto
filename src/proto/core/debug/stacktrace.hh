#pragma once

#include <stdio.h>
#include <execinfo.h>

namespace proto {
namespace debug {

// TODO(kacper): DEMANGLE 
// https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
static void stacktrace() {
    char **strings;
    size_t i, size;
    enum Constexpr { MAX_SIZE = 1024 };
    void *array[MAX_SIZE];
    size = backtrace(array, MAX_SIZE);
    strings = backtrace_symbols(array, size);
    for (i = 0; i < size; i++)
        printf("%s\n", strings[i]);
    puts("");
    free(strings);
}

} // namespace proto
} // namespace debug
