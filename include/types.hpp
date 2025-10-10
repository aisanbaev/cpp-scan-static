#pragma once

#include <algorithm>
#include <cstddef>
#include <tuple>

namespace stdx::details {

// Шаблонный класс, хранящий C-style строку фиксированной длины
template <std::size_t N>
struct fixed_string {
    char data[N] = {};

    constexpr fixed_string(const char (&str)[N]) {
        std::copy_n(str, N, data);
    }

    template <std::size_t M>
    constexpr fixed_string(const char (&str)[M]) requires (M <= N) {
        std::copy_n(str, M - 1, data);
    }

    constexpr fixed_string(const char* begin, const char* end) {
        const std::size_t len = end - begin;
        std::copy_n(begin, (len < N ? len : N), data);
    }

    constexpr std::size_t size() const { return N; }
};


// Шаблонный класс, хранящий fixed_string достаточной длины для хранения ошибки парсинга
struct parse_error : fixed_string<50> {
    template <std::size_t M>
    constexpr parse_error(const char (&str)[M]) : fixed_string<50>(str) {}
};

// Шаблонный класс для хранения результатов парсинга
template <typename... Ts>
struct scan_result {
    std::tuple<Ts...> data;

    constexpr scan_result(Ts... args) : data(args...) {}

    constexpr const std::tuple<Ts...>& values() const {
        return data;
    }
};

} // namespace stdx::details
