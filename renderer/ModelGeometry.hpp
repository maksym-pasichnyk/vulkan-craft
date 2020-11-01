#pragma once

#include <string>
#include <optional>

struct ModelGeometry {
	std::string name;
	std::string parent;
	std::optional<int> texture_width;
	std::optional<int> texture_height;

	std::vector<ModelPart> bones;
};