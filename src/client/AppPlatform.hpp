#pragma once

#include <string>
#include <fstream>
#include <filesystem>

#include "client/util/DescriptorPool.hpp"

struct AppPlatform {
	std::string readAssetFile(const std::filesystem::path& path) {
		std::ifstream ifs("assets" / path);
		if (!ifs) {
			return "";
		}
		return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
	}
};