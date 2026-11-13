#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <array>
#include <charconv>
#include <cstdlib>
#include <initializer_list>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include "global.h"
#include "algorithm.h"
// cross platform console clear
auto clear() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}
template <std::size_t N, typename T = int>
auto parse_argument_list(std::string_view line) {
    std::array<T, N> values;
    if constexpr (std::is_arithmetic_v<T>) {
        values.fill(std::numeric_limits<T>::max());
    } else {
        values.fill(T{});
    }

    std::size_t count = 0;
    bool first_token = true;
    const char* ptr = line.data();
    const char* end = line.data() + line.size();

    while (ptr < end && count < N) {
        while (ptr < end && std::isspace(static_cast<unsigned char>(*ptr))) {
            ++ptr;
        }

        if (ptr == end) break;

        if constexpr (std::is_convertible_v<std::string_view, T> ||
                      std::is_same_v<T, std::string>) {
            const char* token_start = ptr;
            while (ptr < end && !std::isspace(static_cast<unsigned char>(*ptr))) {
                ++ptr;
            }
            std::string_view token(token_start, ptr - token_start);
            values[count++] = T(token);
            if constexpr (std::is_same_v<T, std::string>) {
                if (first_token && count < N) {
                    while (ptr < end && std::isspace(static_cast<unsigned char>(*ptr))) {
                        ++ptr;
                    }
                    if (ptr < end) {
                        values[count++] = std::string(ptr, end);
                        break;
                    }
                }
            }
            first_token = false;
        }
        else if constexpr (std::is_arithmetic_v<T>) {
            T value{};
            auto [next_ptr, ec] = std::from_chars(ptr, end, value);

            if (ec == std::errc{} && next_ptr != ptr &&
                (next_ptr == end || std::isspace(static_cast<unsigned char>(*next_ptr)))) {
                values[count++] = value;
                ptr = next_ptr;
            } else {
                // Skip invalid token and display warning
                const char* token_start = ptr;
                while (ptr < end && !std::isspace(static_cast<unsigned char>(*ptr))) {
                    ++ptr;
                }
                std::cout << "Warning: invalid numeric token '"
                          << std::string_view(token_start, ptr - token_start)
                          << "'\n";
            }
        }
    }

    return values;
}

template <std::size_t N, typename T = int>
auto parse_argument_list_input() {
    std::string line;

    while (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (!line.empty()) {
            return parse_argument_list<N, T>(line);
        }
    }

    std::array<T, N> values;
    values.fill(T{});
    return values;
}

/*
 * Renders statically, at compile time. So we avoid dynamic allocations
*/
auto render(const char* header, const char* menu_name, std::initializer_list<const char*> options, bool top_level = false) {
    clear();

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << '\n';
    std::cout << '\n' << "---" << header << "---" << '\n' << '\n';
    std::cout << "This program performs formula calculation from lab task 4\n"
             "Choose an option using number. File input and output require additional argument (file name) without quotes\n"
             "When entering input form file, it must be space-separated. \n"
             "Follow further instructions on invalid input combinations\n"
             "\n\n"
    ;
    std::cout << "--" << menu_name << "--" << '\n';
    std::cout << (top_level ? "0: Exit program" : "0: return back") << std::endl;
    std::size_t i = 1;
    for (const auto &opt : options) {
        std::cout << i << ": " << opt << std::endl;
        ++i;
    }
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << '\n';
    // explicitly flush
    std::flush(std::cout);
}
auto account_choice() {
    std::cout << "Your choice: ";
    auto [choice, param] = parse_argument_list_input<2, std::string>();
    int choice_int = -1;
    const char* begin = choice.data();
    const char* end = begin + choice.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(*begin))) ++begin;
    while (end > begin && std::isspace(static_cast<unsigned char>(*(end - 1)))) --end;
    auto [ptr, ec] = std::from_chars(begin, end, choice_int);
    if (ec != std::errc{} || ptr != end) {
        choice_int = -1;
    }
    return std::make_pair(choice_int, param);
}
auto wait() -> void {
    std::cout << "[enter to continue]" << std::endl;
    std::string dummy;
    std::getline(std::cin, dummy);
}
auto output_to_file(const char* param) -> bool {
    if (std::isnan(global::calc_result) || global::results.empty()) {
        std::cout << "Calculate your result first" << std::endl;
        wait();
        return false;
    }
    std::ofstream of(param);
    if (!of.is_open()) {
        std::cout << "Could not output to file " << param << "; Check your permissions or path" << std::endl;
        wait();
        return false;
    }
    // one "x y" pair per line: x_k = x_start + k * step
    long long x = global::x_start;
    for (double y : global::results) {
        of << x << ' ' << y << '\n';
        x += global::step;
    }
    if (!of) {
        std::cout << "Error while writing to file " << param << std::endl;
        wait();
        return false;
    }
    std::cout << "Your results are written successfully" << std::endl;
    wait();
    return true;
}
auto read_input_file(const char* param) -> bool {
    std::ifstream ifs(param);
    if (!ifs) {
        std::cout << "Could not read file '" << param << "': check that it exists and you have permission\n";
        wait();
        return false;
    }
    static constexpr std::array<const char*, 4> names{"x_start", "x_end", "step", "n"};
    std::array<int, 4> v{};
    std::string tok;
    bool ok = true;
    for (std::size_t i = 0; i < 4; ++i) {
        if (!(ifs >> tok)) {
            std::cout << "Error: file has only " << i << " of 4 values; missing '" << names[i]
                      << "'. Expected: x_start x_end step n\n";
            ok = false;
            break;
        }
        const char* b = tok.data();
        const char* e = b + tok.size();
        auto [ptr, ec] = std::from_chars(b, e, v[i]);
        if (ec == std::errc::result_out_of_range) {
            std::cout << "Error: value " << (i + 1) << " ('" << tok << "') for '" << names[i]
                      << "' is out of integer range\n";
            ok = false;
        } else if (ec != std::errc{} || ptr != e) {
            std::cout << "Error: value " << (i + 1) << " ('" << tok << "') for '" << names[i]
                      << "' is not a valid integer\n";
            ok = false;
        }
    }
    if (ok && (ifs >> tok)) {
        std::cout << "Error: unexpected extra value '" << tok << "' after the 4 expected values\n";
        ok = false;
    }
    if (ok) {
        const auto [x_start, x_end, step, n] = v;
        if (step == 0) {
            std::cout << "Error: 'step' (value 3) must not be 0\n";
            ok = false;
        } else if (step > 0 && x_start > x_end) {
            std::cout << "Error: step is positive (" << step << ") but x_start (" << x_start
                      << ") > x_end (" << x_end << "); x_start must be <= x_end\n";
            ok = false;
        } else if (step < 0 && x_start < x_end) {
            std::cout << "Error: step is negative (" << step << ") but x_start (" << x_start
                      << ") < x_end (" << x_end << "); x_start must be >= x_end\n";
            ok = false;
        }
        if (n < 2) {
            std::cout << "Error: 'n' (value 4) is " << n << " but must be at least 2\n";
            ok = false;
        }
    }
    if (!ok) {
        wait();
        return false;
    }
    global::x_start = v[0];
    global::x_end = v[1];
    global::step = v[2];
    global::n = v[3];
    return true;
}
auto take_input() -> bool {
    while (std::cin) {
        render("Algorithm calculator v1.0", "Input Your data", {"Input from file", "Input from console"}, true);
        auto [choice, param] = account_choice();
        switch (choice) {
            case 0:
                return false;
            case 1:
                if (param.empty()) {
                    std::cout << "Enter file path(0 to cancel): ";
                    if (!std::getline(std::cin, param)) return false;
                }
                if (param == "0") {
                    std::cout << "You've canceled input from file\n";
                    wait();
                    continue;
                }
                if (read_input_file(param.c_str())) {
                    std::cout << "Parameters loaded successfully\n";
                    wait();
                    return true;
                }
                break;
            case 2: {
                auto [x_start, x_end, step, n] = input_loop();
                global::x_start = x_start;
                global::x_end = x_end;
                global::step = step;
                global::n = n;
                return true;
            }
            default:
                std::cout << "Invalid choice" << std::endl;
                wait();
        }
    }
    return false;
}
auto enter() -> void {
    while (std::cin) {
        if (global::x_start == global::null || global::x_end == global::null || global::step == global::null || global::n == global::null) {
            if (!take_input()) return;
        }
        render("Algorithm calculator v1.0", "main menu", {"Calculate algorithm", "output to file", "New input"}, true);
        auto [choice, param] = account_choice();
        if (!std::cin) return;
        switch (choice) {
            case 0:
                return;
            case 1: {
                if (global::x_start == global::null || global::x_end == global::null || global::step == global::null || global::n == global::null) {
                    auto [x_start, x_end, step, n] = input_loop();
                    global::x_start = x_start;
                    global::x_end = x_end;
                    global::step = step;
                    global::n = n;
                }
                if (global::step == 0 || global::n < 2 ||
                    (global::step > 0 && global::x_start > global::x_end) ||
                    (global::step < 0 && global::x_start < global::x_end)) {
                    std::cout << "Invalid parameters; enter them again.\n";
                    global::x_start = global::x_end = global::step = global::n = global::null;
                    wait();
                    break;
                }
                global::results = calculate(global::x_start, global::x_end, global::step, global::n);
                global::calc_result = global::results.empty()
                    ? std::nan("")
                    : global::results.back();
                std::cout << "Calculated!" << std::endl;
                // keep the printed results visible: the menu clears the screen
                wait();
                break;
            } case 2: {
                if (std::isnan(global::calc_result)) {
                    std::cout << "Calculate your result first" << std::endl;
                    wait();
                    break;
                }
                if (param.empty()) {
                    std::cout << "File path to output (0 to cancel): ";
                    if (!std::getline(std::cin, param)) return;
                }
                if (param == "0" || param.empty()) {
                    break;
                }
                output_to_file(param.c_str());
                break;
            }
            case 3:
                if (!take_input()) return;
                break;
            default:
                std::cout << "Invalid choice" << std::endl;
                wait();
        }
    }
}
