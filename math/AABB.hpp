#pragma once

struct AABB {
	float minX;
	float minY;
	float minZ;
	float maxX;
	float maxY;
	float maxZ;

	void set(float x1, float y1, float z1, float x2, float y2, float z2) {
		minX = x1;
		minY = y1;
		minZ = z1;
		maxX = x2;
		maxY = y2;
		maxZ = z2;
	}
};
