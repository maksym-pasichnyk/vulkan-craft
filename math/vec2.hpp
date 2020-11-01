#pragma once

#include "vec.hpp"

template <typename T>
struct vec<2, T> {
	using value_type = T;
	using reference = value_type &;
	using const_reference = const value_type &;

	value_type x = value_type(0);
	value_type y = value_type(0);

	constexpr vec() = default;
	constexpr vec(value_type x, value_type y) : x{x}, y{y} {}
	constexpr vec(value_type values[2]) : x{values[0]}, y{values[1]} {}

	constexpr reference operator[](size_t i) & {
		return this->*fields[i];
	}

	constexpr const_reference operator[](size_t i) const & {
		return this->*fields[i];
	}

	constexpr value_type operator[](size_t i) const && {
		return this->*fields[i];
	}

private:
	inline static constexpr float vec::* const fields[2] {
		&vec::x,
		&vec::y
	};

public:
	inline static const vec Zero{0, 0};
	inline static const vec One{1, 1};
	inline static const vec Right{1, 0};
	inline static const vec Left{-1, 0};
	inline static const vec Up{0, 1};
	inline static const vec Down{0, -1};
};

using Vector2 = vec<2, float>;
using Vector2i = vec<2, int>;