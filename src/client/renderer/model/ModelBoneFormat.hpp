#pragma once

#include <vector>
#include <optional>

#include "ModelCubeFormat.hpp"

#include "util/math/vec3.hpp"

struct ModelBoneFormat {
	std::string name;
	std::optional<std::string> parent;

	Vector3 pivot;
	std::optional<Vector3> bind_pose_rotation;
	std::optional<Vector3> rotation;

	bool neverRender = false;
	bool mirror = false;
	bool reset = false;

	std::vector<ModelCubeFormat> cubes;
};