#pragma once

#include <vector>
#include <tuple>
auto input_loop() -> std::tuple<int, int, int, int>;
auto calculate(int x_start, int x_end, int step, int n) -> std::vector<double>;