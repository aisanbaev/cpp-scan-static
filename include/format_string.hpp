#pragma once

#include <expected>
#include "types.hpp"
#include <array>
#include <cstddef> 

namespace stdx::details {

// Шаблонный класс для хранения форматирующей строчки и ее особенностей
template <auto Str>
class format_string {
public:
    static constexpr auto source = Str;

    static consteval std::expected<size_t, parse_error> get_number_placeholders() {
        constexpr size_t N = Str.size();
        if (!N)
            return 0;
        size_t placeholder_count = 0;
        size_t pos = 0;
        const size_t size = N - 1; // -1 для игнорирования нуль-терминатора

        while (pos < size) {
            // Пропускаем все символы до '{'
            if (Str.data[pos] != '{') {
                ++pos;
                continue;
            }

            // Проверяем незакрытый плейсхолдер
            if (pos + 1 >= size) {
                return std::unexpected(parse_error{"Unclosed last placeholder"});
            }

            // Начало плейсхолдера
            ++placeholder_count;
            ++pos;

            // Проверка спецификатора формата
            if (Str.data[pos] == '%') {
                ++pos;
                if (pos >= size) {
                    return std::unexpected(parse_error{"Unclosed last placeholder"});
                }

                // Проверяем допустимые спецификаторы
                const char spec = Str.data[pos];
                constexpr char valid_specs[] = {'d', 'u', 's'};
                bool valid = false;

                for (const char s : valid_specs) {
                    if (spec == s) {
                        valid = true;
                        break;
                    }
                }

                if (!valid) {
                    return std::unexpected(parse_error{"Invalid specifier."});
                }
                ++pos;
            }

            // Проверяем закрывающую скобку
            if (pos >= size || Str.data[pos] != '}') {
                return std::unexpected(parse_error{"\'}\' hasn't been found in appropriate place"});
            }
            ++pos;
        }

        return placeholder_count;
    }

    // Статическое поле number_placeholders
    static constexpr size_t number_placeholders = [] {
        constexpr auto result = get_number_placeholders();
        static_assert(result.has_value(), "get_number_placeholders failed");
        return result.value();
    }();

    static consteval auto get_placeholder_positions() {
        constexpr size_t N = Str.size();
        std::array<std::pair<size_t, size_t>, number_placeholders> positions{};
        size_t idx = 0;
        size_t pos = 0;
        const size_t size = N - 1;

        while (pos < size) {
            if (Str.data[pos] != '{') {
                ++pos;
                continue;
            }

            size_t start = pos;
            ++pos; // пропускаем '{'

            if (pos < size && Str.data[pos] == '%') {
                ++pos; // пропускаем '%'
                ++pos; // пропускаем спецификатор
            }

            if (pos < size && Str.data[pos] == '}') {
                positions[idx++] = {start, pos};
                ++pos; // пропускаем '}'
            }
        }

        return positions;
    }

    // Статическое поле placeholder_positions
    static constexpr auto placeholder_positions = get_placeholder_positions();
};

namespace literals {

template <stdx::details::fixed_string Str>
consteval auto operator"" _fs() {
    return stdx::details::format_string<Str>{};
}

} // namespace literals

} // namespace stdx::details
