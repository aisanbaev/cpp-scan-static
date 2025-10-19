#pragma once

#include "parse.hpp"
#include "format_string.hpp"
#include "types.hpp"

namespace stdx {

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