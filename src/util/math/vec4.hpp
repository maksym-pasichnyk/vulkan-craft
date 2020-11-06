#pragma once

#include "vec.hpp"

template <typename T>
struct vec<4, T> {
	using value_type = T;
	using reference = value_type &;
	using const_reference = const value_type &;

	value_type x = value_type(0);
	value_type y = value_type(0);
	value_type z = value_type(0);
	value_type w = value_type(0);

	constexpr vec() = default;
	constexpr vec(value_type x, value_type y, value_type z, value_type w) : x{x}, y{y}, z{z}, w{w} {}
	constexpr vec(value_type values[4]) : x{values[0]}, y{values[1]}, z{values[2]}, w{values[4]} {}

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
	inline static constexpr float vec::* const fields[4] {
		&vec::x,
		&vec::y,
		&vec::z,
		&vec::w,
	};
};

using Vector4 = vec<4, float>;
using Vector4i = vec<4, int>;