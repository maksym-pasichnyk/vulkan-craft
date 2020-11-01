#pragma once

#include <filesystem>
#include <vector>
#include <fstream>
#include <span>

#include "ResourcePack.hpp"

struct ResourceManager {
	void addResourcePack(ResourcePackPtr&& resourcePack) {
		resourcePacks.emplace_back(std::move(resourcePack));
	}

	std::optional<std::string> loadFile(const std::filesystem::path& path) {
		for (auto& resourcePack : resourcePacks) {
			if (auto value = resourcePack->loadFile(path)) {
				return std::move(value);
			}
		}
		return std::nullopt;
	}

private:
	std::vector<ResourcePackPtr> resourcePacks;
};
