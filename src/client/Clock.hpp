#pragma once

#include <chrono>

struct Clock {
	using time_point = std::chrono::high_resolution_clock::time_point;
	using duration = std::chrono::high_resolution_clock::duration;

	Clock() = default;

	void reset() {
		time = std::chrono::high_resolution_clock::now();
		delta = std::chrono::nanoseconds{};
	}

	void update() {
		auto now = std::chrono::high_resolution_clock::now();
		delta = now - time;
		time = now;
	}

	[[nodiscard]] float deltaSeconds() const {
		return std::chrono::duration<double>(delta).count();
	}

private:
	time_point time;
	duration delta;
};
