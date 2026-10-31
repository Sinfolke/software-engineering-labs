#include <cmath>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>
#include "corelib.h"
auto input_loop() -> std::tuple<int, int, int, int> {
    auto null = std::numeric_limits<int>::max();
    int x_start = null,
        x_end = null,
        step = null,
        n = null;

    while (std::cin) {
        if (step == null || step == 0) {
            gather_v("step", step, "Must not be zero");
            if (!std::cin) return {x_start, x_end, step, n};
            if (step == std::numeric_limits<int>::max()) step = null;
        } else if (n == null || n < 2) {
            gather_v("n", n, "must be within range [2, ∞)");
            if (!std::cin) return {x_start, x_end, step, n};
            if (n == std::numeric_limits<int>::max()) n = null;
        } else if (x_start == null && x_end == null) {
            gather_v("x_start", x_start, "must be lower than x_end");
            if (!std::cin) return {x_start, x_end, step, n};
            gather_v("x_end", x_end, "must be higher than x_start");
            if (!std::cin) return {x_start, x_end, step, n};
        } else if ((step > 0 && x_start > x_end) || (step < 0 && x_start < x_end)) {
            std::cout << "incorrect combination of x_start, x_end and step. Overwrite one of the following values:\n";
            output_candidate_for_input("x_start", "must be lower than x_end", true);
            output_candidate_for_input("x_end", "must be higher than x_start", true);
            output_candidate_for_input("step", "must not be zero", true);
            std::cout << "Your input: ";
            if (!parse_into_candidate(std::make_tuple("x_start", "x_end", "step"), std::tie(x_start, x_end, step)))
                std::cout << "Invalid input; use x_start:<integer>, x_end:<integer>, or step:<integer>.\n";
            if (!std::cin) return {x_start, x_end, step, n};
        } else break;
    }
    return std::make_tuple(x_start, x_end, step, n);
}

auto calculate(int x_start, int x_end, int step, int n) -> std::vector<double> {
    auto x = x_start;
    std::vector<double> results;
    while ((step > 0 && x <= x_end) || (step < 0 && x >= x_end)) {
        double y = x >= 0 ? 1.0 : 0.0;
        if (x >= 0) {
            for (int i = 0; i <= n; ++i) {
                double y_j = 1;
                for (int j = 1; j < n; ++j) {
                    y_j *= i + (std::pow(x, 2) / (i + j)) - 3*j;
                }
                y *= y_j;
            }
        } else {
            for (int i = 1; i <= n; ++i) {
                // Promote before adding: x + i can overflow as an int for
                // inputs near INT_MAX, even though the result is a double.
                const double term = static_cast<double>(x) + i;
                y += term * term;
            }
            y += 5.0 * x;
        }
        if (y == 0.0) y = 0.0; // avoid printing "-0" (a product with a zero factor)
        std::cout << "x: " << x << "; y: " << y << '\n';
        results.push_back(y);
        if ((step > 0 && x > std::numeric_limits<int>::max() - step) ||
            (step < 0 && x < std::numeric_limits<int>::min() - step)) break;
        x += step;
    }
    return results;
}

