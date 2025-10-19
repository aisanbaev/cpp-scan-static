#pragma once

#include <algorithm>
#include <cstddef>
#include <tuple>

namespace stdx::details {

constinit const std::size_t PARSE_ERROR_MAX_SIZE = 43;

// Шаблонный класс, хранящий C-style строку фиксированной длины
template <std::size_t N>
struct fixed_string {
    char data[N] = {};

    consteval fixed_string(const char (&str)[N]) {
        std::copy_n(str, N, data);
    }

    template <std::size_t M>
    consteval fixed_string(const char (&str)[M]) requires (M <= N) {
        std::copy_n(str, M - 1, data);
    }

    consteval fixed_string(const char* begin, const char* end) {
        const std::size_t len = end - begin;
        std::copy_n(begin, (len < N ? len : N), data);
    }

    consteval std::size_t size() const { return N; }
};

// Шаблонный класс для хранения ошибки парсинга
struct parse_error : fixed_string<PARSE_ERROR_MAX_SIZE> {
    template <std::size_t M>
    constexpr parse_error(const char (&str)[M]) : fixed_string<PARSE_ERROR_MAX_SIZE>(str) {}
};

// Шаблонный класс для хранения результатов парсинга
template <typename... Ts>
struct scan_result {
    std::tuple<Ts...> data;

    consteval scan_result(Ts... args) : data(args...) {}

    consteval const std::tuple<Ts...>& values() const {
        return data;
    }
};

} // namespace stdx::details
