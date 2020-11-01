#pragma once

#include <vector>

struct ModelPart {
	std::string name;
	std::string parent;
	bool neverRender = false;

	std::vector<ModelPartBox> cubes;
};