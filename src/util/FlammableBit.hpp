#pragma once

#include <type_traits>

enum class FlammableBit {
    Up = 1,
    Down = 2,
    North = 4,
    South = 8,
    East = 16,
    West = 32
};

inline constexpr auto operator|(FlammableBit __lhs, FlammableBit __rhs) -> FlammableBit {
    using base = std::underlying_type<FlammableBit>::type;
    return FlammableBit(base(__lhs) | base(__rhs));
}

inline constexpr auto operator&(FlammableBit __lhs, FlammableBit __rhs) -> FlammableBit {
    using base = std::underlying_type<FlammableBit>::type;
    return FlammableBit(base(__lhs) & base(__rhs));
}


inline constexpr auto operator|=(FlammableBit& __lhs, FlammableBit __rhs) -> FlammableBit& {
    using base = std::underlying_type<FlammableBit>::type;
    return __lhs = FlammableBit(base(__lhs) | base(__rhs));
}