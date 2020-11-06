#pragma once

#include <string>
#include <optional>

#include "ParsedGeometryBoneNode.hpp"

#include "util/math/vec3.hpp"

struct ParsedModelGeometry {
	std::string name;
	std::optional<std::string> parent;

	int visible_bounds_width;
	int visible_bounds_height;
	Vector3 visible_bounds_offset;

	std::optional<int> texture_width;
	std::optional<int> texture_height;

	std::vector<ParsedGeometryBoneNode> bones;
};