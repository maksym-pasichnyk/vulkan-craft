#pragma once

struct ResourcePack {
	ResourcePack(std::filesystem::path path) : basePath(std::move(path)) {}

	std::filesystem::path getFullPath(const std::filesystem::path& path) {
		return basePath / path;
	}

	inline bool contains(const std::string& path) {
		const auto file_path = basePath / path;
		return std::filesystem::exists(file_path);
	}

	inline auto loadFile(const std::filesystem::path& path) -> std::optional<std::string> {
		const auto filePath = getFullPath(path);
		if (std::filesystem::exists(filePath)) {
			std::string bytes;
			bytes.resize(std::filesystem::file_size(filePath));

			std::ifstream file(filePath, std::ios::binary);
			file.read(bytes.data(), bytes.size());
			file.close();

			return std::move(bytes);
		}
		return std::nullopt;
	}

private:
	std::filesystem::path basePath;
};

using ResourcePackPtr = std::unique_ptr<ResourcePack>;
