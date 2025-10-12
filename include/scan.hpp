#pragma once

#include "parse.hpp"
#include "format_string.hpp"
#include "types.hpp"

namespace stdx {

// Вспомогательная функция для определения типа по спецификатору
template<auto Fmt, std::size_t I, typename DefaultT>
consteval auto get_type_for_placeholder() {
    constexpr char spec = details::get_specifier<Fmt, I>();
    
    if constexpr (spec == 'd') {
        return int{};
    } else if constexpr (spec == 'u') {
        return unsigned{};
    } else if constexpr (spec == 's') {
        return std::string_view{};
    } else {
        return DefaultT{};
    }
}

// Главная функция scan
template <details::format_string fmt, details::fixed_string source, typename... Ts>
consteval details::scan_result<Ts...> scan() {
    static_assert(fmt.number_placeholders == sizeof...(Ts), 
        "Number of placeholders must match number of types");

    return []<std::size_t... Is>(std::index_sequence<Is...>) {
        return details::scan_result<Ts...>(
            details::parse_input<Is, fmt, source, Ts>()...
        );
    }(std::make_index_sequence<sizeof...(Ts)>{});
}

} // namespace stdx