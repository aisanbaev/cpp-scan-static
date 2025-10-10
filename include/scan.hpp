#pragma once

#include <tuple>
#include "parse.hpp"
#include "format_string.hpp"
#include "types.hpp"

namespace stdx {

// Концепт для проверки, что все типы поддерживаются
template<typename... Ts>
concept AllSupportedScanTypes = (details::SupportedScanType<Ts> && ...);

// Вспомогательная функция для определения типа по спецификатору
template<auto Fmt, std::size_t I, typename DefaultT>
consteval auto get_type_for_placeholder() {
    constexpr auto& placeholder = Fmt.placeholder_positions[I];
    constexpr std::string_view fmt_sv(Fmt.source.data, Fmt.source.size() - 1);
    constexpr auto placeholder_sv = fmt_sv.substr(placeholder.first, placeholder.second - placeholder.first + 1);
    
    constexpr bool has_d_spec = placeholder_sv.find("{%d}") != std::string_view::npos;
    constexpr bool has_u_spec = placeholder_sv.find("{%u}") != std::string_view::npos;
    constexpr bool has_s_spec = placeholder_sv.find("{%s}") != std::string_view::npos;
    
    static_assert((has_d_spec ? 1 : 0) + (has_u_spec ? 1 : 0) + (has_s_spec ? 1 : 0) <= 1,
        "Multiple specifiers in one placeholder");
    
    if constexpr (has_d_spec) {
        return int{};
    } else if constexpr (has_u_spec) {
        return unsigned{};
    } else if constexpr (has_s_spec) {
        return std::string_view{};
    } else {
        return DefaultT{};
    }
}

// Главная функция scan с концептом
template <details::format_string fmt, details::fixed_string source, AllSupportedScanTypes... Ts>
consteval details::scan_result<Ts...> scan() {
    static_assert(fmt.number_placeholders == sizeof...(Ts), 
        "Number of placeholders must match number of types");

    return []<std::size_t... Is>(std::index_sequence<Is...>) {
        // Определяем фактические типы для парсинга на основе спецификаторов
        using ActualTypes = std::tuple<
            decltype(get_type_for_placeholder<fmt, Is, std::tuple_element_t<Is, std::tuple<Ts...>>>())...
        >;
        
        return details::scan_result<Ts...>(
            details::parse_input<Is, fmt, source, std::tuple_element_t<Is, ActualTypes>>()...
        );
    }(std::make_index_sequence<sizeof...(Ts)>{});
}

} // namespace stdx