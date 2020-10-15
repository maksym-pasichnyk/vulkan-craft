#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 coords;

	Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz)
		: position{x, y, z}
		, normal{nx, ny, nz}
		, coords{u, v} {}
};

struct VertexBuilder {
	std::vector<int> indices;
	std::vector<Vertex> vertices;

	void addQuad(int a1, int b1, int c1, int a2, int b2, int c2) {
		int idx = vertices.size();

		indices.reserve(6);

		indices.push_back(idx + a1);
		indices.push_back(idx + b1);
		indices.push_back(idx + c1);

		indices.push_back(idx + a2);
		indices.push_back(idx + b2);
		indices.push_back(idx + c2);
	}

	void clear() {
		indices.clear();
		vertices.clear();
	}
};