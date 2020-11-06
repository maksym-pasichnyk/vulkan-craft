#pragma once

#include <glm/glm.hpp>

#include <vector>

#include "ModelBox.hpp"

struct ModelRenderer {
	glm::vec3 pivot;
	std::vector<ModelBox> cubes;
};