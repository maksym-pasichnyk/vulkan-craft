#pragma once

#include <string>
#include <optional>

#include "ModelBoneFormat.hpp"

#include "util/math/vec3.hpp"

struct ModelFormat {
	std::string name;
	std::optional<std::string> parent;

	int visible_bounds_width;
	int visible_bounds_height;
	Vector3 visible_bounds_offset;

	int texture_width;
	int texture_height;

	std::vector<ModelBoneFormat> bones;
};