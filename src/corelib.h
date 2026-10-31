#pragma once
#include <cctype>
#include <cstddef>
#include <utility>
#include <iostream>
#include <charconv>
#include <limits>
#include <string_view>
#include <string>
#include <tuple>
#include <type_traits>


inline auto output_candidate_for_input(const char* i, const char* hint = "", bool new_line = false) -> void {
    std::cout << "Input '" << i << "' [" << hint << "]:" << (new_line ? "\n" : "");
}
template<typename T>
auto gather_v(const char* i, T &v, const char* hint = "") -> void {
    while (true) {
        output_candidate_for_input(i, hint);
        std::string line;
        if (!std::getline(std::cin, line)) return;

        T parsed{};
        const auto* begin = line.data();
        const auto* end = begin + line.size();
        while (begin < end && std::isspace(static_cast<unsigned char>(*begin))) ++begin;
        while (end > begin && std::isspace(static_cast<unsigned char>(*(end - 1)))) --end;
        const auto [ptr, ec] = std::from_chars(begin, end, parsed);
        if (ec == std::errc{} && ptr == end) {
            v = parsed;
            return;
        }
        std::cout << "Invalid integer; try again.\n";
    }
}
template <typename... Hints, typename... Vars>
bool parse_into_candidate(const std::tuple<Hints...>& hints, std::tuple<Vars&...> vars) {
    static_assert(sizeof...(Hints) == sizeof...(Vars), "Number of hints must match number of variables.");

    std::string v;
    if (!std::getline(std::cin, v)) return false;

    bool matched = false;
    std::string_view input{v};

    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        ([&] {
            if (matched || !input.starts_with(std::get<Is>(hints))) return;
                const auto prefix = std::string_view(std::get<Is>(hints));
                if (input.size() <= prefix.size() || input[prefix.size()] != ':') return;
                auto value = input.substr(prefix.size() + 1);
                int parsed{};
                const auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed);
                if (ec == std::errc{} && ptr == value.data() + value.size()) {
                    std::get<Is>(vars) = parsed;
                    matched = true;
                }
        }(), ...);
    }(std::make_index_sequence<sizeof...(Hints)>{});

    return matched;
}
