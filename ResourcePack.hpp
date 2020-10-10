#pragma once

#include <filesystem>
#include <vector>
#include <fstream>
#include <span>

struct ResourcePack {
	std::filesystem::path basePath;

	explicit ResourcePack(std::filesystem::path assets) : basePath(std::move(assets)) {}

	inline bool contains(const std::string& path) {
		const auto file_path = basePath / path;
		return std::filesystem::exists(file_path);
	}

	inline bool loadFile(const std::string& path, std::vector<char>& bytes) const {
		const auto file_path = basePath / path;
		if (std::filesystem::exists(file_path)) {
			bytes.resize(std::filesystem::file_size(file_path));

			std::ifstream file(file_path, std::ios::binary);
			file.read(bytes.data(), bytes.size());
			file.close();

			return true;
		}
		return false;
	}

	template <typename Fn>
	inline bool loadFile(const std::string& path, Fn&& fn) const {
		const auto file_path = basePath / path;
		if (std::filesystem::exists(file_path)) {
			std::vector<char> bytes(std::filesystem::file_size(file_path));

			std::ifstream file(file_path, std::ios::binary);
			file.read(bytes.data(), bytes.size());
			file.close();

			fn(std::span(bytes.data(), bytes.size()));
			return true;
		}
		return false;
	}
};