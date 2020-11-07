#pragma once

#include <unordered_map>
#include <variant>

#include "util/math/vec2.hpp"
#include "util/math/vec3.hpp"

struct ModelFaceFormat {
	Vector2 uv;
};

struct ModelCubeFormat {
	Vector3 origin;
	Vector3 size;

	std::variant<Vector2, std::unordered_map<std::string, ModelFaceFormat>> uv;
};