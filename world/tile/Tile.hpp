#pragma once

#include "renderer/TextureUVCoordinateSet.hpp"

#include "math/AABB.hpp"

struct Tile {
	virtual ~Tile() = default;

	virtual void getVisualShape(char data, AABB& shape) {
		shape = visualShape;
	}

	virtual void getVisualShape(int data, const TilePos& tilePos, AABB& shape) {
		shape = visualShape;
	}

	void setVisualShape(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z) {
		visualShape.set(min_x, min_y, min_z, max_x, max_y, max_z);
	}

	virtual TextureUVCoordinateSet getTexture(unsigned char side, int data) {
		return {0, 0, 1, 1};
	}

private:
	AABB visualShape;
};
