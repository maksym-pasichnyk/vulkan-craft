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

	bool loadFile(const std::filesystem::path& path, std::vector<char>& bytes) {
		for (auto& resourcePack : resourcePacks) {
			if (resourcePack->loadFile(path, bytes)) {
				return true;
			}
		}
		return false;
	}

	template <typename Fn>
	bool loadFile(const std::filesystem::path& path, Fn&& fn) {
		std::vector<char> bytes;
		for (auto& resourcePack : resourcePacks) {
			if (resourcePack->loadFile(path, bytes)) {
				fn(std::span(bytes.data(), bytes.size()));
				return true;
			}
		}
		return false;
	}

private:
	std::vector<ResourcePackPtr> resourcePacks;
};
