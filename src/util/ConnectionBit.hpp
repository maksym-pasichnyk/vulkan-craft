#pragma once

#include <type_traits>

enum class ConnectionBit {
    None = 0,
    West = 1,
    North = 2,
    East = 4,
    South = 8,
};

inline constexpr auto operator&&(ConnectionBit __lhs, ConnectionBit __rhs) -> bool {
    using base = std::underlying_type<ConnectionBit>::type;
    return (base(__lhs) & base(__rhs)) != base(0);
}

inline constexpr auto operator|(ConnectionBit __lhs, ConnectionBit __rhs) -> ConnectionBit {
    using base = std::underlying_type<ConnectionBit>::type;
    return ConnectionBit(base(__lhs) | base(__rhs));
}