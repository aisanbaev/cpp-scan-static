#pragma once

#include <charconv>
#include <expected>
#include <cstdint>
#include <system_error>
#include <string_view>
#include <type_traits>
#include "format_string.hpp"
#include "types.hpp"

namespace stdx::details {

template<typename T>
concept SupportedIntegerType = 
    std::is_same_v<std::remove_cv_t<T>, int8_t>   || 
    std::is_same_v<std::remove_cv_t<T>, int16_t>  || 
    std::is_same_v<std::remove_cv_t<T>, int32_t>  || 
    std::is_same_v<std::remove_cv_t<T>, int64_t>  ||
    std::is_same_v<std::remove_cv_t<T>, uint8_t>  || 
    std::is_same_v<std::remove_cv_t<T>, uint16_t> || 
    std::is_same_v<std::remove_cv_t<T>, uint32_t> || 
    std::is_same_v<std::remove_cv_t<T>, uint64_t>;

template<typename T>
concept SupportedStringType = 
    std::is_same_v<std::remove_cv_t<T>, std::string_view>;

template<typename T>
concept SupportedScanType = 
    SupportedIntegerType<T> || SupportedStringType<T>;

// Парсинг целых чисел
template<SupportedIntegerType T>
consteval std::expected<T, parse_error> parse_value(std::string_view str) {
    T value{};
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    
    if (ec != std::errc{}) {
        return std::unexpected(parse_error{"Failed to parse integer"});
    }
    
    if (ptr != str.data() + str.size()) {
        return std::unexpected(parse_error{"Extra characters after integer"});
    }
    
    return value;
}

// Парсинг строк
template<SupportedStringType T>
consteval std::expected<T, parse_error> parse_value(std::string_view str) {
    return str;
}

// Шаблонная функция, возвращающая пару позиций в строке с исходными данными, соотвествующих I-ому плейсхолдеру
template<int I, stdx::details::format_string Fmt, fixed_string Source>
consteval auto get_current_source_for_parsing() {
    static_assert(I >= 0 && I < Fmt.number_placeholders, "Invalid placeholder index");

    constexpr auto to_sv = [](const auto& fs) {
        return std::string_view(fs.data, fs.size() - 1);
    };

    constexpr auto fmt_sv = to_sv(Fmt.source);
    constexpr auto src_sv = to_sv(Source);
    constexpr auto& positions = Fmt.placeholder_positions;

    // Получаем границы текущего плейсхолдера в формате
    constexpr auto pos_i = positions[I];
    constexpr size_t fmt_start = pos_i.first, fmt_end = pos_i.second;

    // Находим начало в исходной строке
    constexpr auto src_start = [&]{
        if constexpr (I == 0) {
            return fmt_start;
        } else {
            // Находим конец предыдущего плейсхолдера в исходной строке
            constexpr auto prev_bounds = get_current_source_for_parsing<I-1, Fmt, Source>();
            const auto prev_end = prev_bounds.second;

            // Получаем разделитель между текущим и предыдущим плейсхолдерами
            constexpr auto prev_fmt_end = positions[I-1].second;
            constexpr auto sep = fmt_sv.substr(prev_fmt_end + 1, fmt_start - (prev_fmt_end + 1));

            // Ищем разделитель после предыдущего значения
            auto pos = src_sv.find(sep, prev_end);
            return pos != std::string_view::npos ? pos + sep.size() : src_sv.size();
        }
    }();

    // Находим конец в исходной строке
    constexpr auto src_end = [&]{
        // Получаем разделитель после текущего плейсхолдера
        if constexpr(fmt_end == (fmt_sv.size() - 1)) {
            return src_sv.size();
        }
        constexpr auto sep = fmt_sv.substr(fmt_end + 1,
            (I < Fmt.number_placeholders - 1)
                ? positions[I+1].first - (fmt_end + 1)
                : fmt_sv.size() - (fmt_end + 1));
        // Ищем разделитель после текущего значения
        constexpr auto pos = src_sv.find(sep, src_start);
        return pos != std::string_view::npos ? pos : src_sv.size();
    }();
    return std::pair{src_start, src_end};
}

// Шаблонная функция, выполняющая преобразования исходных данных в конкретный тип на основе I-го плейсхолдера
template<int I, auto Fmt, auto Source, SupportedScanType T>
consteval auto parse_input() {
    static_assert(I >= 0 && I < Fmt.number_placeholders, "Invalid placeholder index");
    
    constexpr auto bounds = get_current_source_for_parsing<I, Fmt, Source>();
    constexpr size_t start = bounds.first;
    constexpr size_t end = bounds.second;
    
    // Проверяем, что границы текущего плейсхолдера корректны
    static_assert(start <= end, "Invalid parsing bounds");
    static_assert(end <= Source.size() - 1, "Parsing bounds exceed source size");

    constexpr std::string_view src_sv(Source.data, Source.size() - 1);
    constexpr auto data_sv = src_sv.substr(start, end - start);
    
    constexpr auto parsing_result = parse_value<T>(data_sv);
    static_assert(parsing_result.has_value(), "Parsing failed");
    
    return parsing_result.value();
}

} // namespace stdx::details