#pragma once

#include <cstddef>
#include <cstdint>

#include <type_traits>

template <typename T>
class span {
public:
	constexpr span(std::nullptr_t) noexcept : ptr(nullptr), len(0) {}

	constexpr span(T &v) noexcept : ptr(&v), len(1) {}

	template <typename U>
	constexpr span(span<U> in) noexcept : len(std::size(in)), ptr(std::data(in)) {}

	constexpr span(T *ptr, uint32_t len) noexcept : ptr(ptr), len(len) {}

//	template<typename U, size_t _Size>
//	constexpr span(std::array<U, _Size> const &in) noexcept : _size(_Size), _data(std::data(in)) {}

//	template<typename U>
//	constexpr span(std::vector<U> /*const */&in) noexcept : _size(std::size(in)), _data(std::data(in)) {}

//		template<typename U>
//		constexpr span(U&& in) noexcept : m_count(std::size(in)), m_ptr(std::data(in)) {}

//	template<size_t _Size>
//	constexpr span(/*const */T (&data)[_Size]) noexcept : _size(_Size), _data(data) {}

	template <typename U, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::decay_t<U>>>>
	constexpr span(U& in) noexcept : len(std::size(in)), ptr(std::data(in)) {}

//	template<size_t N, typename = std::enable_if_t<std::is_const_v<>>>
//	constexpr span(const value_type (&data)[N]) noexcept : m_count(N), m_ptr(data) {}

	/*template<class Allocator = std::allocator<typename std::remove_const<T>::type>>
	span(std::vector<typename std::remove_const<T>::type, Allocator> &data) noexcept
			: m_count(static_cast<uint32_t>(data.size())), m_ptr(data.data()) {
	}

	template<class Allocator = std::allocator<typename std::remove_const<T>::type>>
	span(std::vector<typename std::remove_const<T>::type, Allocator> const &data) noexcept
			: m_count(static_cast<uint32_t>(data.size())), m_ptr(data.data()) {
	}

	span(std::initializer_list<value_type> const &data) noexcept
			: m_count(static_cast<uint32_t>(data.end() - data.begin())), m_ptr(data.begin()) {
	}*/

	const T *begin() const noexcept {
		return ptr;
	}

	const T *end() const noexcept {
		return ptr + len;
	}

	const T &front() const noexcept {
		VULKAN_HPP_ASSERT(len && ptr);
		return *ptr;
	}

	const T &back() const noexcept {
		VULKAN_HPP_ASSERT(len && ptr);
		return *(ptr + len - 1);
	}

	bool empty() const noexcept {
		return (len == 0);
	}

	uint32_t size() const noexcept {
		return len;
	}

	T *data() noexcept {
		return ptr;
	}

	const T *data() const noexcept {
		return ptr;
	}

	T& operator[](size_t i) noexcept {
		return ptr[i];
	}

	const T& operator[](size_t i) const noexcept {
		return ptr[i];
	}

	span<T> sub(size_t i) {
//		assert(i < len);
		return span(ptr + i, len - i);
	}

private:
	T *ptr;
	uint32_t len;
};