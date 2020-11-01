#pragma once

#include "Tile.hpp"

struct ButtonTile : Tile {
	void getVisualShape(int data, const TilePos& tilePos, AABB& shape) override {
		int rot = data & 7;
		bool powered = (data & 8) > 0;

		float f = 0.375F;
		float f1 = 0.625F;
		float f2 = 0.1875F;
		float f3 = 0.125F;

		if (powered) f3 = 0.0625F;

		if (rot == 1)
			shape.set(0.0F, f, 0.5F - f2, f3, f1, 0.5F + f2);
		else if (rot == 2)
			shape.set(1.0F - f3, f, 0.5F - f2, 1.0F, f1, 0.5F + f2);
		else if (rot == 3)
			shape.set(0.5F - f2, f, 0.0F, 0.5F + f2, f1, f3);
		else if (rot == 4)
			shape.set(0.5F - f2, f, 1.0F - f3, 0.5F + f2, f1, 1.0F);
		else if (rot == 5)
			shape.set(f, 0.0F, 0.5F - f2, f1, f3, 0.5F + f2);
		else if (rot == 6)
			shape.set(f, 1.0F - f3, 0.5F - f2, f1, 1.0F, 0.5F + f2);
	}
};