#pragma once

#include <string>
#include <fstream>

#include "DescriptorPool.hpp"

struct AppPlatform {
	std::string readAssetFile(const std::string& path) {
		std::ifstream ifs(std::string("assets/") + path);
		if (!ifs) {
			return "";
		}
		return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
	}
};