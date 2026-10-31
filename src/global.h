#pragma once
#include <limits>
#include <vector>

namespace global {
    inline auto null = std::numeric_limits<int>::max();
    extern double calc_result;

    extern int x_start,
               x_end,
               step,
               n;
    extern std::vector<double> results;
}
