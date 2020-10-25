#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <array>

struct PositionTextureVertex {
	float x;
	float y;
	float z;
	float u;
	float v;
};

struct TexturedQuad {
	std::array<PositionTextureVertex, 4> vertices;
	glm::vec3 normal;

//	TexturedQuad(std::array<PositionTextureVertex, 4> _vertices, float u0, float v0, float u1, float v1, float texture_width, float texture_height, glm::vec3 normal)
//		: vertices(_vertices), normal(normal) {
//
//		vertices[0].u = u0 /*/ texture_width*/; vertices[0].v = v1 /*/ texture_height*/;
//		vertices[1].u = u0 /*/ texture_width*/; vertices[1].v = v0 /*/ texture_height*/;
//		vertices[2].u = u1 /*/ texture_width*/; vertices[2].v = v0 /*/ texture_height*/;
//		vertices[3].u = u1 /*/ texture_width*/; vertices[3].v = v1 /*/ texture_height*/;
//	}
};

struct ModelBox {
	std::array<std::optional<TexturedQuad>, 6> quads;
};

struct ModelRenderer {
	glm::vec3 pivot;
	std::vector<ModelBox> cubes;
};