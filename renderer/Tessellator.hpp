#pragma once

#include <vector>

struct Tessellator {
	std::vector<int> indices;
	std::vector<Vertex> vertices;
    Vector3 _normal;
    Vector3 _offset;
    Color32 _color;

    inline static Tessellator* instance() {
    	static Tessellator self;
    	return &self;
    }

    inline void normal(Vector3 normal) {
        _normal = normal;
    }

	inline void normal(float x, float y, float z) {
        _normal.x = x;
        _normal.y = y;
        _normal.z = z;
    }

	inline void addOffset(float x, float y, float z) {
        _offset.x += x;
        _offset.y += y;
        _offset.z += z;
    }

	inline void setOffset(float x, float y, float z) {
        _offset.x = x;
        _offset.y = y;
        _offset.z = z;
    }

    inline void color(Color32 color) {
        _color = color;
    }

    inline void color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 0xFF) {
        _color.r = r;
        _color.g = g;
        _color.b = b;
        _color.a = a;
    }

    inline void quad() {
		quad(0, 1, 2, 0, 2, 3);
    }

    inline void quadInv() {
		quad(0, 2, 1, 0, 3, 2);
    }

	inline void quad(int a, int b, int c) {
		int idx = vertices.size();

		indices.reserve(3);

		indices.push_back(idx + a);
		indices.push_back(idx + b);
		indices.push_back(idx + c);
    }

	void quad(int a1, int b1, int c1, int a2, int b2, int c2) {
		int idx = vertices.size();

		indices.reserve(6);

		indices.push_back(idx + a1);
		indices.push_back(idx + b1);
		indices.push_back(idx + c1);

		indices.push_back(idx + a2);
		indices.push_back(idx + b2);
		indices.push_back(idx + c2);
	}

	inline void vertexUV(float x, float y, float z, float u, float v) {
		vertices.emplace_back(x + _offset.x, y + _offset.y, z + _offset.z, u, v, _normal.x, _normal.y, _normal.z);
    }

	inline void vertexUV(float x, float y, float z, float u, float v, unsigned char light) {
		vertices.emplace_back(x + _offset.x - 0.5f, y + _offset.y - 0.5f, z + _offset.z - 0.5f, u, v, _normal.x, _normal.y, _normal.z);
    }
};