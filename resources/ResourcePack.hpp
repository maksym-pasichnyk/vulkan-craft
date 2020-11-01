#pragma once

struct ResourcePack {
	ResourcePack(std::filesystem::path path) : basePath(std::move(path)) {}

	std::filesystem::path getFullPath(const std::filesystem::path& path) const {
		return basePath / path;
	}

	inline bool contains(const std::string& path) {
		const auto file_path = basePath / path;
		return std::filesystem::exists(file_path);
	}

	inline bool loadFile(const std::filesystem::path& path, std::vector<char>& bytes) const {
		const auto filePath = getFullPath(path);
		if (std::filesystem::exists(filePath)) {
			bytes.resize(std::filesystem::file_size(filePath));

			std::ifstream file(filePath, std::ios::binary);
			file.read(bytes.data(), bytes.size());
			file.close();

			return true;
		}
		return false;
	}

	template <typename Fn>
	inline bool loadFile(const std::filesystem::path& path, Fn&& fn) const {
		const auto filePath = getFullPath(path);
		if (std::filesystem::exists(filePath)) {
			std::vector<char> bytes(std::filesystem::file_size(filePath));

			std::ifstream file(filePath, std::ios::binary);
			file.read(bytes.data(), bytes.size());
			file.close();

			fn(std::span(bytes.data(), bytes.size()));
			return true;
		}
		return false;
	}

private:
	std::filesystem::path basePath;
};

using ResourcePackPtr = std::unique_ptr<ResourcePack>;
