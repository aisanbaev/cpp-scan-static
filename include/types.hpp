#pragma once
#include <cstddef>
#include <tuple>

namespace stdx::details {

// Шаблонный класс, хранящий C-style строку фиксированной длины
template <std::size_t N>
struct fixed_string {
    char data[N] = {};

    constexpr fixed_string(const char (&str)[N]) {
        for (std::size_t i = 0; i < N; ++i) {
            data[i] = str[i];
        }
    }

    template <std::size_t M>
    constexpr fixed_string(const char (&str)[M]) 
        requires (M <= N) 
    {
        for (std::size_t i = 0; i < M - 1; ++i) {
            data[i] = str[i];
        }
    }

    constexpr fixed_string(const char* begin, const char* end) {
        std::size_t i = 0;
        while (begin != end && i < N) {
            data[i++] = *begin++;
        }
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
