#pragma once

#include <unordered_map>

struct ModelPartBox {
	std::unordered_map<std::string, TexturedQuad> quads;
};