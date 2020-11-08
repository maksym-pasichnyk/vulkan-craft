#pragma once

#include "client/util/stb_image.hpp"

#include <span>

struct NativeImage {
	void* pixels = nullptr;
	int width = 0;
	int height = 0;
	int channels = 0;

	static NativeImage read(std::span<char> bytes) {
		auto data = reinterpret_cast<const unsigned char *>(bytes.data());

		int width, height, channels;
		auto pixels = stbi_load_from_memory(data, bytes.size(), &width, &height, &channels, 0);

		return NativeImage{
			.pixels = pixels,
			.width = width,
			.height = height,
			.channels = channels
		};
	}
};