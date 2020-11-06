#pragma once

#include <memory>

template <typename T>
struct Handle {
	Handle(std::unique_ptr<T>& ref) : ref(ref.get()) {}

	template <typename U>
	Handle(std::unique_ptr<U>& ref) : ref(ref.get()) {}

	T* operator->() const noexcept {
		return ref;
	}

	T* get() const noexcept {
		return ref;
	}
private:
	T* ref;
};