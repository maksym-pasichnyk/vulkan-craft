#pragma once

#include "Tessellator.hpp"

#include "math/AABB.hpp"

struct BlockTessellator {
	Tessellator* tessellator = Tessellator::instance();
	AABB bounds;

//    bool useForcedUV = false;
//	TextureUVCoordinateSet forcedUV;

	int rotTop = 0;
	int rotBottom = 0;
	int rotNorth = 0;
	int rotSouth = 0;
	int rotEast = 0;
	int rotWest = 0;

	void setRenderBounds(int rotation, float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
		bounds.minX = minX;
		bounds.minY = minY;
		bounds.minZ = minZ;
		bounds.maxX = maxX;
		bounds.maxY = maxY;
		bounds.maxZ = maxZ;
	}

	void setRotatedBounds(int rotation, float x1, float y1, float z1, float x2, float y2, float z2) {
		bounds.set(x1, y1, z1, x2, y2, z2);

		switch (rotation) {
        case 0: // south
        	bounds.minX = x1;
        	bounds.minY = y1;
        	bounds.minZ = z1;
        	bounds.maxX = x2;
        	bounds.maxY = y2;
        	bounds.maxZ = z2;
			break;
        case 1: // east
        	bounds.minX = 1 - z2;
        	bounds.minY = y1;
        	bounds.minZ = x1;
        	bounds.maxX = 1 - z1;
        	bounds.maxY = y2;
        	bounds.maxZ = x2;
        	break;
        case 2: // north
        	bounds.minX = 1 - x2;
        	bounds.minY = y1;
        	bounds.minZ = 1 - z2;
        	bounds.maxX = 1 - x1;
        	bounds.maxY = y2;
        	bounds.maxZ = 1 - z1;
			break;
        case 3: // west
        	bounds.minX = z1;
        	bounds.minY = y1;
        	bounds.minZ = 1 - x2;
        	bounds.maxX = z2;
        	bounds.maxY = y2;
        	bounds.maxZ = 1 - x1;
			break;
//        case 4: // up
//        	bounds.minX = x1;
//        	bounds.minY = 1 - z2;
//        	bounds.minZ = y1;
//        	bounds.maxX = x2;
//        	bounds.maxY = 1 - z1;
//        	bounds.maxZ = y2;
//			break;
//        case 5: // down
//        	bounds.minX = x1;
//        	bounds.minY = z1;
//        	bounds.minZ = 1 - y2;
//        	bounds.maxX = x2;
//        	bounds.maxY = z2;
//        	bounds.maxZ = 1 - y1;
//			break;
		}

//        switch (rotation) {
//        case 2: // south
//        	bounds.minX = x1;
//        	bounds.minY = y1;
//        	bounds.minZ = z1;
//        	bounds.maxX = x2;
//        	bounds.maxY = y2;
//        	bounds.maxZ = z2;
//			break;
//        case 3: // north
//        	bounds.minX = 1 - x2;
//        	bounds.minY = y1;
//        	bounds.minZ = 1 - z2;
//        	bounds.maxX = 1 - x1;
//        	bounds.maxY = y2;
//        	bounds.maxZ = 1 - z1;
//			break;
//        case 4: // east
//        	bounds.minX = 1 - z2;
//        	bounds.minY = y1;
//        	bounds.minZ = x1;
//        	bounds.maxX = 1 - z1;
//        	bounds.maxY = y2;
//        	bounds.maxZ = x2;
//			break;
//        case 5: // west
//        	bounds.minX = z1;
//        	bounds.minY = y1;
//        	bounds.minZ = 1 - x2;
//        	bounds.maxX = z2;
//        	bounds.maxY = y2;
//        	bounds.maxZ = 1 - x1;
//			break;
////        case 4: // up
////        	bounds.minX = x1;
////        	bounds.minY = 1 - z2;
////        	bounds.minZ = y1;
////        	bounds.maxX = x2;
////        	bounds.maxY = 1 - z1;
////        	bounds.maxZ = y2;
////			break;
////        case 5: // down
////        	bounds.minX = x1;
////        	bounds.minY = z1;
////        	bounds.minZ = 1 - y2;
////        	bounds.maxX = x2;
////        	bounds.maxY = z2;
////        	bounds.maxZ = 1 - y1;
////			break;
//		}
	}

	void setRotatedBounds(int rotation, const AABB& aabb) {
		bounds = aabb;

		switch (rotation) {
        case 3:
        	bounds.minX = 1 - aabb.maxX;
        	bounds.minY = aabb.minY;
        	bounds.minZ = 1 - aabb.maxZ;
        	bounds.maxX = 1 - aabb.minX;
        	bounds.maxY = aabb.maxY;
        	bounds.maxZ = 1 - aabb.minZ;
			break;
        case 2:
        	bounds.minX = aabb.minX;
        	bounds.minY = aabb.minY;
        	bounds.minZ = aabb.minZ;
        	bounds.maxX = aabb.maxX;
        	bounds.maxY = aabb.maxY;
        	bounds.maxZ = aabb.maxZ;
			break;
        case 4:
        	bounds.minX = aabb.minZ;
        	bounds.minY = aabb.minY;
        	bounds.minZ = aabb.minX;
        	bounds.maxX = aabb.maxZ;
        	bounds.maxY = aabb.maxY;
        	bounds.maxZ = aabb.maxX;
			break;
        case 5:
        	bounds.minX = 1 - aabb.maxZ;
        	bounds.minY = aabb.minY;
        	bounds.minZ = 1 - aabb.maxX;
        	bounds.maxX = 1 - aabb.minZ;
        	bounds.maxY = aabb.maxY;
        	bounds.maxZ = 1 - aabb.minX;
			break;
//        case 4:
//        	bounds.minX = aabb.minX;
//        	bounds.minY = 1 - aabb.maxZ;
//        	bounds.minZ = aabb.minY;
//        	bounds.maxX = aabb.maxX;
//        	bounds.maxY = 1 - aabb.minZ;
//        	bounds.maxZ = aabb.maxY;
//			break;
//        case 5:
//        	bounds.minX = aabb.minX;
//        	bounds.minY = aabb.minZ;
//        	bounds.minZ = 1 - aabb.maxY;
//        	bounds.maxX = aabb.maxX;
//        	bounds.maxY = aabb.maxZ;
//        	bounds.maxZ = 1 - aabb.minY;
//			break;
		}
	}

	void setRenderBox(float x1, float y1, float z1, float x2, float y2, float z2) {
		bounds.set(x1 / 16.0f, y1 / 16.0f, z1 / 16.0f, x2 / 16.0f, y2 / 16.0f, z2 / 16.0f);
	}

	void setRotatedBox(int rotation, float x1, float y1, float z1, float x2, float y2, float z2) {
		setRotatedBounds(rotation, x1 / 16.0f, y1 / 16.0f, z1 / 16.0f, x2 / 16.0f, y2 / 16.0f, z2 / 16.0f);
	}

    inline void tessellateBlockInWorld(Tile* tile, const TilePos& tilePos) {
        Vector3 p1 { bounds.minX, bounds.minY, bounds.minZ };
	 	Vector3 p2 { bounds.minX, bounds.minY, bounds.maxZ };
        Vector3 p3 { bounds.maxX, bounds.minY, bounds.maxZ };
        Vector3 p4 { bounds.maxX, bounds.minY, bounds.minZ };
		Vector3 p5 { bounds.minX, bounds.maxY, bounds.minZ };
        Vector3 p6 { bounds.minX, bounds.maxY, bounds.maxZ };
        Vector3 p7 { bounds.maxX, bounds.maxY, bounds.maxZ };
        Vector3 p8 { bounds.maxX, bounds.maxY, bounds.minZ };

		tessellator->normal(0, 1, 0);
		tessellator->quad();
		rotateFace(tilePos, p5, p6, p7, p8, 0, rotTop, tile->getTexture(0, 0),
				bounds.minX,
				bounds.minZ,
				bounds.maxX,
				bounds.maxZ
		);

		tessellator->normal(0, -1, 0);
		tessellator->quad();
		rotateFace(tilePos, p2, p1, p4, p3, 0, rotBottom, tile->getTexture(1, 0),
				bounds.minX,
				1 - bounds.maxZ,
				bounds.maxX,
				1 - bounds.minZ
		);

		tessellator->normal(0, 0, 1);
		tessellator->quad();
		rotateFace(tilePos, p3, p7, p6, p2, 0, rotNorth, tile->getTexture(2, 0),
				1 - bounds.maxX,
				bounds.minY,
				1 - bounds.minX,
				bounds.maxY
		);

		tessellator->normal(0, 0, -1);
		tessellator->quad();
		rotateFace(tilePos, p1, p5, p8, p4, 0, rotSouth, tile->getTexture(3, 0),
				bounds.minX,
				bounds.minY,
				bounds.maxX,
				bounds.maxY
		);

		tessellator->normal(1, 0, 0);
		tessellator->quad();
		rotateFace(tilePos, p4, p8, p7, p3, 0, rotEast, tile->getTexture(4, 0),
				bounds.minZ,
				bounds.minY,
				bounds.maxZ,
				bounds.maxY
		);

		tessellator->normal(-1, 0, 0);
		tessellator->quad();
		rotateFace(tilePos, p2, p6, p5, p1, 0, rotWest, tile->getTexture(5, 0),
				1 - bounds.maxZ,
				bounds.minY,
				1 - bounds.minZ,
				bounds.maxY
		);
    }

    inline void tessellateAngledInWorld(Tile* tile, const TilePos& tilePos, ModelTransform& transform) {
        Vector3 p1 { bounds.minX, bounds.minY, bounds.minZ };
	 	Vector3 p2 { bounds.minX, bounds.minY, bounds.maxZ };
        Vector3 p3 { bounds.maxX, bounds.minY, bounds.maxZ };
        Vector3 p4 { bounds.maxX, bounds.minY, bounds.minZ };
		Vector3 p5 { bounds.minX, bounds.maxY, bounds.minZ };
        Vector3 p6 { bounds.minX, bounds.maxY, bounds.maxZ };
        Vector3 p7 { bounds.maxX, bounds.maxY, bounds.maxZ };
        Vector3 p8 { bounds.maxX, bounds.maxY, bounds.minZ };

        apply(&transform, p1, p2, p3, p4);
        apply(&transform, p5, p6, p7, p8);

		tessellator->normal(0, 1, 0);
		tessellator->quad();
		rotateFace(tilePos, p5, p6, p7, p8, 0, rotTop, tile->getTexture(0, 0),
				bounds.minX,
				bounds.minZ,
				bounds.maxX,
				bounds.maxZ
		);

		tessellator->normal(0, -1, 0);
		tessellator->quad();
		rotateFace(tilePos, p2, p1, p4, p3, 0, rotBottom, tile->getTexture(1, 0),
				bounds.minX,
				1 - bounds.maxZ,
				bounds.maxX,
				1 - bounds.minZ
		);

		tessellator->normal(0, 0, 1);
		tessellator->quad();
		rotateFace(tilePos, p3, p7, p6, p2, 0, rotNorth, tile->getTexture(2, 0),
				1 - bounds.maxX,
				bounds.minY,
				1 - bounds.minX,
				bounds.maxY
		);

		tessellator->normal(0, 0, -1);
		tessellator->quad();
		rotateFace(tilePos, p1, p5, p8, p4, 0, rotSouth, tile->getTexture(3, 0),
				bounds.minX,
				bounds.minY,
				bounds.maxX,
				bounds.maxY
		);

		tessellator->normal(1, 0, 0);
		tessellator->quad();
		rotateFace(tilePos, p4, p8, p7, p3, 0, rotEast, tile->getTexture(4, 0),
				bounds.minZ,
				bounds.minY,
				bounds.maxZ,
				bounds.maxY
		);

		tessellator->normal(-1, 0, 0);
		tessellator->quad();
		rotateFace(tilePos, p2, p6, p5, p1, 0, rotWest, tile->getTexture(5, 0),
				1 - bounds.maxZ,
				bounds.minY,
				1 - bounds.minZ,
				bounds.maxY
		);
    }

	inline void _renderUp(const TilePos& tilePos, const TextureUVCoordinateSet& texture) {
		Vector3 p1 { bounds.minX, bounds.maxY, bounds.minZ };
        Vector3 p2 { bounds.minX, bounds.maxY, bounds.maxZ };
        Vector3 p3 { bounds.maxX, bounds.maxY, bounds.maxZ };
        Vector3 p4 { bounds.maxX, bounds.maxY, bounds.minZ };

		tessellator->normal(0, 1, 0);
		rotateFace(tilePos, p1, p2, p3, p4, 0, rotTop, texture,
				bounds.minX,
				bounds.minZ,
				bounds.maxX,
				bounds.maxZ
		);
	}

	inline void _renderDown(const TilePos& tilePos, const TextureUVCoordinateSet& texture) {
	 	Vector3 p1 { bounds.minX, bounds.minY, bounds.maxZ };
        Vector3 p2 { bounds.minX, bounds.minY, bounds.minZ };
        Vector3 p3 { bounds.maxX, bounds.minY, bounds.minZ };
        Vector3 p4 { bounds.maxX, bounds.minY, bounds.maxZ };

		tessellator->normal(0, -1, 0);
		rotateFace(tilePos, p1, p2, p3, p4, 0, rotBottom, texture,
				bounds.minX,
				1 - bounds.maxZ,
				bounds.maxX,
				1 - bounds.minZ
		);
	}

	inline void _renderNorth(const TilePos& tilePos, const TextureUVCoordinateSet& texture) {
        Vector3 p1 { bounds.maxX, bounds.minY, bounds.maxZ };
        Vector3 p2 { bounds.maxX, bounds.maxY, bounds.maxZ };
        Vector3 p3 { bounds.minX, bounds.maxY, bounds.maxZ };
	 	Vector3 p4 { bounds.minX, bounds.minY, bounds.maxZ };

		tessellator->normal(0, 0, 1);
		rotateFace(tilePos, p1, p2, p3, p4, 0, rotNorth, texture,
				1 - bounds.maxX,
				bounds.minY,
				1 - bounds.minX,
				bounds.maxY
		);
	}

	inline void _renderSouth(const TilePos& tilePos, const TextureUVCoordinateSet& texture) {
		Vector3 p1 { bounds.minX, bounds.minY, bounds.minZ };
		Vector3 p2 { bounds.minX, bounds.maxY, bounds.minZ };
        Vector3 p3 { bounds.maxX, bounds.maxY, bounds.minZ };
        Vector3 p4 { bounds.maxX, bounds.minY, bounds.minZ };

		tessellator->normal(0, 0, -1);
		rotateFace(tilePos, p1, p2, p3, p4, 0, rotSouth, texture,
				bounds.minX,
				bounds.minY,
				bounds.maxX,
				bounds.maxY
		);
	}

	inline void _renderEast(const TilePos& tilePos, const TextureUVCoordinateSet& texture) {
        Vector3 p1 { bounds.maxX, bounds.minY, bounds.minZ };
        Vector3 p2 { bounds.maxX, bounds.maxY, bounds.minZ };
        Vector3 p3 { bounds.maxX, bounds.maxY, bounds.maxZ };
        Vector3 p4 { bounds.maxX, bounds.minY, bounds.maxZ };

		tessellator->normal(1, 0, 0);
		rotateFace(tilePos, p1, p2, p3, p4, 0, rotEast, texture,
				bounds.minZ,
				bounds.minY,
				bounds.maxZ,
				bounds.maxY
		);
	}

	inline void _renderWest(const TilePos& tilePos, const TextureUVCoordinateSet& texture) {
	 	Vector3 p1 { bounds.minX, bounds.minY, bounds.maxZ };
        Vector3 p2 { bounds.minX, bounds.maxY, bounds.maxZ };
		Vector3 p3 { bounds.minX, bounds.maxY, bounds.minZ };
		Vector3 p4 { bounds.minX, bounds.minY, bounds.minZ };

		tessellator->normal(-1, 0, 0);
		rotateFace(tilePos, p1, p2, p3, p4, 0, rotWest, texture,
				1 - bounds.maxZ,
				bounds.minY,
				1 - bounds.minZ,
				bounds.maxY
		);
	}

    inline void tessellateAnvilInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//    	tessellator->setMaterial(material);

		int rotation = 2;

		setRotatedBox(rotation, 0, 0, 4, 16, 1, 12);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 1, 0, 3, 15, 1, 4);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 1, 0, 12, 15, 1, 13);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 1, 1, 4, 15, 4, 12);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 4, 4, 5, 12, 5, 12);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 6, 5, 5, 10, 10, 12);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 2, 10, 4, 14, 16, 12);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 14, 11, 4, 16, 15, 12);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 0, 11, 4, 2, 15, 12);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 3, 11, 3, 13, 15, 4);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 3, 11, 12, 13, 15, 13);
		tessellateBlockInWorld(tile, tilePos);
    }

	inline void tessellateSpawnerInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);

		setRenderBox(1, 1, 1, 15, 15, 15);
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateBrewingStand(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);

		int rotation = 2;

		setRotatedBox(rotation, 0, 0, 0, 16, 2, 16);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 6, 2, 0, 10, 15, 2);
		tessellateBlockInWorld(tile, tilePos);

//		bounds.set(0.25, 0.9375, 0.125, 0.75, 0.9375, 0.625);
//		tessellateUp(tile, tilePos);
//
//		bounds.set(0.125, 0.9375, 0.3125, 0.875, 0.9375, 0.4375);
//		tessellateUp(tile, tilePos);
//
//		bounds.set(0.4375, 0.9375, 0.625, 0.5625, 0.9375, 0.75);
//		tessellateUp(tile, tilePos);

//		bounds.set(3 / 16.0f, 2 / 16.0f, 4 / 16.0f, 13 / 16.0f, 14.8f / 16.0f, 4 / 16.0f);
//		tessellateNorth(tile, tilePos);
//		tessellateSouth(tile, tilePos);
	}

	inline void tessellateButtonInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);

		float f = 6;
		float f1 = 10;
		float f2 = 3;

//		bool powered = false;
		float f3 = /*powered ? 1 :*/ 2;

		int data = 0;

		switch (data) {
		case 0:
			setRenderBox(f, 16 - f3, 8 - f2, f1, 16, 8 + f2);
			break;
		case 1:
			setRenderBox(0, f, 8 - f2, f3, f1, 8 + f2);
			break;
		case 2:
			setRenderBox(16 - f3, f, 8 - f2, 16, f1, 8 + f2);
			break;
		case 3:
			setRenderBox(8 - f2, f, 16 - f3, 8 + f2, f1, 16);
			break;
		case 4:
			setRenderBox(8 - f2, f, 0, 8 + f2, f1, f3);
			break;
		case 5:
			setRenderBox(f, 0, 8 - f2, f1, f3, 8 + f2);
			break;
		}
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateFenceInWorld(Tile* tile, const TilePos& tilePos) {
		ConnectionBit connections = ConnectionBit::South | ConnectionBit::East | ConnectionBit::North | ConnectionBit::West;

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setTile(getTile(mb_cfg, texture));
//		tessellator->setMaterial(material);

		bounds.set(0.3f, 0, 0.3f, 0.7f, 1, 0.7f);
		tessellateBlockInWorld(tile, tilePos);

		if (connections && ConnectionBit::South) {
			bounds.set(0.4f, 0.3f, 0, 0.6f, 0.5f, 0.3f);
			tessellateBlockInWorld(tile, tilePos);

			bounds.set(0.4f, 0.7f, 0, 0.6f, 0.9f, 0.3f);
			tessellateBlockInWorld(tile, tilePos);
		}

		if (connections && ConnectionBit::East) {
			bounds.set(0.7f, 0.3f, 0.4f, 1.0f, 0.5f, 0.6f);
			tessellateBlockInWorld(tile, tilePos);

			bounds.set(0.7f, 0.7f, 0.4f, 1.0f, 0.9f, 0.6f);
			tessellateBlockInWorld(tile, tilePos);
		}

		if (connections && ConnectionBit::North) {
			bounds.set(0.4f, 0.3f, 0.7f, 0.6f, 0.5f, 1.0f);
			tessellateBlockInWorld(tile, tilePos);

			bounds.set(0.4f, 0.7f, 0.7f, 0.6f, 0.9f, 1.0f);
			tessellateBlockInWorld(tile, tilePos);
		}

		if (connections && ConnectionBit::West) {
			bounds.set(0, 0.3f, 0.4f, 0.3f, 0.5f, 0.6f);
			tessellateBlockInWorld(tile, tilePos);

			bounds.set(0, 0.7f, 0.4f, 0.3f, 0.9f, 0.6f);
			tessellateBlockInWorld(tile, tilePos);
		}
	}

	inline void tessellateWallInWorld(Tile* tile, const TilePos& tilePos) {
		ConnectionBit connections = ConnectionBit::South | ConnectionBit::East | ConnectionBit::North | ConnectionBit::West;

    	bounds.set(0.2f, 0, 0.2f, 0.8f, 1, 0.8f);
		tessellateBlockInWorld(tile, tilePos);

		if (connections && ConnectionBit::South) {
			bounds.set(0.3f, 0, 0, 0.7f, 0.8f, 0.2f);
			tessellateBlockInWorld(tile, tilePos);
		}

		if (connections && ConnectionBit::East) {
			bounds.set(0.8f, 0, 0.3f, 1.0f, 0.8f, 0.7f);
			tessellateBlockInWorld(tile, tilePos);
		}

		if (connections && ConnectionBit::North) {
			bounds.set(0.3f, 0, 0.8f, 0.7f, 0.8f, 1.0f);
			tessellateBlockInWorld(tile, tilePos);
		}

		if (connections && ConnectionBit::West) {
			bounds.set(0, 0, 0.3f, 0.2f, 0.8f, 0.7f);
			tessellateBlockInWorld(tile, tilePos);
		}
    }

	inline void tessellateFenceGateInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);

		int rotation = 2;

		switch (/*m_res->cc_block->additional_data*/0) {
		case 0: {
			setRotatedBox(rotation, 0, 4, 7, 2, 15, 9);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 14, 4, 7, 16, 15, 9);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 2, 5, 7, 14, 8, 9);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 2, 11, 7, 14, 14, 9);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 6, 8, 7, 10, 11, 9);
			tessellateBlockInWorld(tile, tilePos);

			break;
		}
		case 1: {
			setRotatedBox(rotation, 0, 4, 8, 2, 15, 10);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 0, 5, 14, 2, 14, 16);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 0, 5, 10, 2, 8, 14);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 0, 11, 10, 2, 14, 14);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 14, 4, 8, 16, 15, 10);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 14, 5, 14, 16, 14, 16);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 14, 5, 10, 16, 8, 14);
			tessellateBlockInWorld(tile, tilePos);

			setRotatedBox(rotation, 14, 11, 10, 16, 14, 14);
			tessellateBlockInWorld(tile, tilePos);

			break;
		}
		}
	}

	inline void tessellateTorchInWorld(Tile* tile, const TilePos& tilePos) {
	    tessellator->color(0xFF, 0xFF, 0xFF);

		ModelTransform transform {};

		int data = 1;

		switch (data) {
		case 1:
			transform.rotZ = glm::radians(-22.5f);
        	setRenderBox(-1, 3.5f, 7, 1.5f, 13.5f, 9.5f);
			break;
		case 2:
			transform.rotZ = glm::radians(22.5f);
        	setRotatedBox(3, -1, 3.5f, 7, 1.5f, 13.5f, 9.5f);
			break;
		case 3:
			transform.rotX = glm::radians(-22.5f);
        	setRotatedBox(5, -1, 3.5f, 7, 1.5f, 13.5f, 9.5f);
			break;
		case 4:
			transform.rotX = glm::radians(22.5f);
			setRotatedBox(4, -1, 3.5f, 7, 1.5f, 13.5f, 9.5f);
			break;
		case 5:
			setRenderBox(6.75f, 0, 6.75f, 9.25f, 0.625, 9.25f);
			break;
		}

		tessellateAngledInWorld(tile, tilePos, transform);
	}

	inline void tessellatePaintingInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);

		int rotation = 3;
		setRotatedBounds(rotation, 0, 0, 0.9, 1, 1, 1);
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateLilyPadInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);
//		bounds.set(0.07, 0.01, 0.07, 0.93, 0.01, 0.93);
		bounds.set(0, 0.01, 0, 1, 0.01, 1);

		auto lilypad = tile->getTexture(0, 0);

		tessellator->quad();
		tessellator->quadInv();
		_renderUp(tilePos, lilypad);
	}

	inline void tessellateRailInWorld(Tile* tile, const TilePos& tilePos) {
		int x = tilePos.x;
		int y = tilePos.y;
		int z = tilePos.z;

		tessellator->color(0xFF, 0xFF, 0xFF);
//    tessellator->setMaterial(material);

		int rotation = 2;
		int ascending[4]{0, 0, 0, 0};

////    TileCoordinateSet tile;
//    switch ((RailDirection)m_res->cc_block->additional_data) {
//    case RailDirection::EAST_WEST:
////        tile = getTile(mb_cfg, texture);
//        rotation = 1;
//        break;
//    case RailDirection::NORTH_SOUTH:
////        tile = getTile(mb_cfg, texture);
//        rotation = 0;
//        break;
//    case RailDirection::SOUTH_EAST:
////        tile = getTile(mb_cfg, texture, Direction::East);
//        rotation = 0;
//        break;
//    case RailDirection::SOUTH_WEST:
////        tile = getTile(mb_cfg, texture, Direction::East);
//        rotation = 3;
//        break;
//    case RailDirection::NORTH_WEST:
////        tile = getTile(mb_cfg, texture, Direction::East);
//        rotation = 2;
//        break;
//    case RailDirection::NORTH_EAST:
////        tile = getTile(mb_cfg, texture, Direction::East);
//        rotation = 1;
//        break;
//    case RailDirection::ASCENDING_EAST:
////        tile = getTile(mb_cfg, texture);
//        ascending[2] = 1;
//        ascending[3] = 1;
//        rotation = 1;
//        break;
//    case RailDirection::ASCENDING_WEST:
////        tile = getTile(mb_cfg, texture);
//        ascending[0] = 1;
//        ascending[1] = 1;
//        rotation = 1;
//        break;
//    case RailDirection::ASCENDING_SOUTH:
////        tile = getTile(mb_cfg, texture);
//        ascending[0] = 1;
//        ascending[3] = 1;
//        break;
//    case RailDirection::ASCENDING_NORTH:
////        tile = getTile(mb_cfg, texture);
//        ascending[1] = 1;
//        ascending[2] = 1;
//        rotation = 0;
//        break;
//
//    default:
//        tile = getTile(mb_cfg, texture);
//        break;
//    }

		TextureUVCoordinateSet rails = {0, 0, 1, 1};

		const float coords[]{
				rails.getInterpolatedU(0), rails.getInterpolatedV(0),
				rails.getInterpolatedU(0), rails.getInterpolatedV(1),
				rails.getInterpolatedU(1), rails.getInterpolatedV(1),
				rails.getInterpolatedU(1), rails.getInterpolatedV(0)
		};

		auto i0 = ((rotation + 0) & 3) << 1;
		auto i1 = ((rotation + 1) & 3) << 1;
		auto i2 = ((rotation + 2) & 3) << 1;
		auto i3 = ((rotation + 3) & 3) << 1;

		tessellator->quad();
//    tessellator->quadInv();
		tessellator->vertexUV(0 + x, ascending[0] + y, 0 + z, coords[i0], coords[i0 | 1], 15);
		tessellator->vertexUV(0 + x, ascending[1] + y, 1 + z, coords[i1], coords[i1 | 1], 15);
		tessellator->vertexUV(1 + x, ascending[2] + y, 1 + z, coords[i2], coords[i2 | 1], 15);
		tessellator->vertexUV(1 + x, ascending[3] + y, 0 + z, coords[i3], coords[i3 | 1], 15);
	}

	inline void tessellateFlowerInWorld(Tile* tile, const TilePos& tilePos) {
		int x = tilePos.x;
		int y = tilePos.y;
		int z = tilePos.z;

		auto zero = Vector3::Zero;
		Vector3 b{zero};
		b = zero;

		TextureUVCoordinateSet flower = {0, 0, 1, 1};

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);

		auto min_u = flower.getInterpolatedU(0);
		auto min_v = flower.getInterpolatedV(0);
		auto max_u = flower.getInterpolatedU(1);
		auto max_v = flower.getInterpolatedV(1);

		tessellator->quad();
		tessellator->vertexUV(0 + x, 0 + y, 0 + z, min_u, min_v, 15);
		tessellator->vertexUV(0 + x, 1 + y, 0 + z, min_u, max_v, 15);
		tessellator->vertexUV(1 + x, 1 + y, 1 + z, max_u, max_v, 15);
		tessellator->vertexUV(1 + x, 0 + y, 1 + z, max_u, min_v, 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(0 + x, 0 + y, 0 + z, min_u, min_v, 15);
//		tessellator->vertexUV(0 + x, 1 + y, 0 + z, min_u, max_v, 15);
//		tessellator->vertexUV(1 + x, 1 + y, 1 + z, max_u, max_v, 15);
//		tessellator->vertexUV(1 + x, 0 + y, 1 + z, max_u, min_v, 15);

		tessellator->quad();
		tessellator->vertexUV(0 + x, 0 + y, 1 + z, min_u, min_v, 15);
		tessellator->vertexUV(0 + x, 1 + y, 1 + z, min_u, max_v, 15);
		tessellator->vertexUV(1 + x, 1 + y, 0 + z, max_u, max_v, 15);
		tessellator->vertexUV(1 + x, 0 + y, 0 + z, max_u, min_v, 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(0 + x, 0 + y, 1 + z, min_u, min_v, 15);
//		tessellator->vertexUV(0 + x, 1 + y, 1 + z, min_u, max_v, 15);
//		tessellator->vertexUV(1 + x, 1 + y, 0 + z, max_u, max_v, 15);
//		tessellator->vertexUV(1 + x, 0 + y, 0 + z, max_u, min_v, 15);

//		if (m_res->cc_block->additional_data & 1) {
//			auto texture = GetTexture(mb_cfg, ItemId::Snow, 0);
//			tessellator->setTile(getTile(mb_cfg, texture));
			bounds.set(0, 0, 0, 1, 0.125f, 1);
		tessellateBlockInWorld(tile, tilePos);
//		}
	}

	inline void tessellatePistonBaseInWorld(PistonBaseTile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);

		tile->getVisualShape(1 | 8, tilePos, bounds);
		tessellateBlockInWorld(tile, tilePos);
	}

//	inline void tessellatePistonArmInWorld(PistonArmTile* tile, const TilePos& tilePos) {
//		int x = tilePos.x;
//		int y = tilePos.y;
//		int z = tilePos.z;
//
//		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);
//
//		int data = 1;
//		int rotation = PistonArmTile::getRotation(data);
//		bool powered = true;
//		float var8 = 0.25F;
//		float var9 = 0.375F;
//		float var10 = 0.625F;
//		float var11 = powered ? 1.0F : 0.5F;
//		float var12 = powered ? 16.0F : 8.0F;
//
//		tile->getVisualShape(data, tilePos, bounds);
////		tessellateBlockInWorld(tile, tilePos);
//
//		switch(rotation) {
//		case 0:
////			rotEast = 3;
////			rotWest = 3;
////			rotSouth = 3;
////			rotNorth = 3;
//			tessellateBlockInWorld(tile, tilePos);///*pos*/);
//			tessellatePistonRodUD(tile, x + 0.375F, x + 0.625F, y + 0.25F, y + 0.25F + var11, z + 0.625F, z + 0.625F, 0.8F, var12);
//			tessellatePistonRodUD(tile, x + 0.625F, x + 0.375F, y + 0.25F, y + 0.25F + var11, z + 0.375F, z + 0.375F, 0.8F, var12);
//			tessellatePistonRodUD(tile, x + 0.375F, x + 0.375F, y + 0.25F, y + 0.25F + var11, z + 0.375F, z + 0.625F, 0.6F, var12);
//			tessellatePistonRodUD(tile, x + 0.625F, x + 0.625F, y + 0.25F, y + 0.25F + var11, z + 0.625F, z + 0.375F, 0.6F, var12);
//			break;
//		case 1:
//			tessellateBlockInWorld(tile, tilePos);//pos);
//			tessellatePistonRodUD(tile, x + 0.375F, x + 0.625F, y - 0.25F + 1.0F - var11, y - 0.25F + 1.0F, z + 0.625F, z + 0.625F, 0.8F, var12);
//			tessellatePistonRodUD(tile, x + 0.625F, x + 0.375F, y - 0.25F + 1.0F - var11, y - 0.25F + 1.0F, z + 0.375F, z + 0.375F, 0.8F, var12);
//			tessellatePistonRodUD(tile, x + 0.375F, x + 0.375F, y - 0.25F + 1.0F - var11, y - 0.25F + 1.0F, z + 0.375F, z + 0.625F, 0.6F, var12);
//			tessellatePistonRodUD(tile, x + 0.625F, x + 0.625F, y - 0.25F + 1.0F - var11, y - 0.25F + 1.0F, z + 0.625F, z + 0.375F, 0.6F, var12);
//			break;
//		case 2:
////			rotSouth = 1;
////			rotNorth = 2;
//			tessellateBlockInWorld(tile, tilePos);//pos);
//			tessellatePistonRodSN(tile, x + 0.375F, x + 0.375F, y + 0.625F, y + 0.375F, z + 0.25F, z + 0.25F + var11, 0.6F, var12);
//			tessellatePistonRodSN(tile, x + 0.625F, x + 0.625F, y + 0.375F, y + 0.625F, z + 0.25F, z + 0.25F + var11, 0.6F, var12);
//			tessellatePistonRodSN(tile, x + 0.375F, x + 0.625F, y + 0.375F, y + 0.375F, z + 0.25F, z + 0.25F + var11, 0.5F, var12);
//			tessellatePistonRodSN(tile, x + 0.625F, x + 0.375F, y + 0.625F, y + 0.625F, z + 0.25F, z + 0.25F + var11, 1.0F, var12);
//			break;
//		case 3:
////			rotSouth = 2;
////			rotNorth = 1;
////			rotTop = 3;
////			rotBottom = 3;
//			tessellateBlockInWorld(tile, tilePos);//pos);
//			tessellatePistonRodSN(tile, x + 0.375F, x + 0.375F, y + 0.625F, y + 0.375F, z - 0.25F + 1.0F - var11, z - 0.25F + 1.0F, 0.6F, var12);
//			tessellatePistonRodSN(tile, x + 0.625F, x + 0.625F, y + 0.375F, y + 0.625F, z - 0.25F + 1.0F - var11, z - 0.25F + 1.0F, 0.6F, var12);
//			tessellatePistonRodSN(tile, x + 0.375F, x + 0.625F, y + 0.375F, y + 0.375F, z - 0.25F + 1.0F - var11, z - 0.25F + 1.0F, 0.5F, var12);
//			tessellatePistonRodSN(tile, x + 0.625F, x + 0.375F, y + 0.625F, y + 0.625F, z - 0.25F + 1.0F - var11, z - 0.25F + 1.0F, 1.0F, var12);
//			break;
//		case 4:
////			rotEast = 1;
////			rotWest = 2;
////			rotTop = 2;
////			rotBottom = 1;
//			tessellateBlockInWorld(tile, tilePos);//pos);
//			tessellatePistonRodEW(tile, x + 0.25F, x + 0.25F + var11, y + 0.375F, y + 0.375F, z + 0.625F, z + 0.375F, 0.5F, var12);
//			tessellatePistonRodEW(tile, x + 0.25F, x + 0.25F + var11, y + 0.625F, y + 0.625F, z + 0.375F, z + 0.625F, 1.0F, var12);
//			tessellatePistonRodEW(tile, x + 0.25F, x + 0.25F + var11, y + 0.375F, y + 0.625F, z + 0.375F, z + 0.375F, 0.6F, var12);
//			tessellatePistonRodEW(tile, x + 0.25F, x + 0.25F + var11, y + 0.625F, y + 0.375F, z + 0.625F, z + 0.625F, 0.6F, var12);
//			break;
//		case 5:
////			rotEast = 2;
////			rotWest = 1;
////			rotTop = 1;
////			rotBottom = 2;
//			tessellateBlockInWorld(tile, tilePos);//pos);
//			tessellatePistonRodEW(tile, x - 0.25F + 1.0F - var11, x - 0.25F + 1.0F, y + 0.375F, y + 0.375F, z + 0.625F, z + 0.375F, 0.5F, var12);
//			tessellatePistonRodEW(tile, x - 0.25F + 1.0F - var11, x - 0.25F + 1.0F, y + 0.625F, y + 0.625F, z + 0.375F, z + 0.625F, 1.0F, var12);
//			tessellatePistonRodEW(tile, x - 0.25F + 1.0F - var11, x - 0.25F + 1.0F, y + 0.375F, y + 0.625F, z + 0.375F, z + 0.375F, 0.6F, var12);
//			tessellatePistonRodEW(tile, x - 0.25F + 1.0F - var11, x - 0.25F + 1.0F, y + 0.625F, y + 0.375F, z + 0.625F, z + 0.625F, 0.6F, var12);
//		}
//	}

	inline void tessellateLanternInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);

		setRenderBox(2.5f, 0.25, 2.5f, 13.5f, 12.5f, 13.5f);
		tessellateBlockInWorld(tile, tilePos);

		setRenderBox(4.5f, 12.5f, 4.5f, 11.5f, 14.5, 11.5f);
		tessellateBlockInWorld(tile, tilePos);

		setRenderBox(7.5f, 14.5, 7.5f, 8.5, 1, 8.5);
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateHopperInWorld(Tile* tile, const TilePos& tilePos) {
		int x = tilePos.x;
		int y = tilePos.y;
		int z = tilePos.z;

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);

		setRenderBox(0, 10, 0, 16, 16, 2);
		tessellateBlockInWorld(tile, tilePos);

		setRenderBox(0, 10, 14, 16, 16, 16);
		tessellateBlockInWorld(tile, tilePos);

		setRenderBox(14, 10, 2, 16, 16, 14);
		tessellateBlockInWorld(tile, tilePos);

		setRenderBox(0, 10, 2, 2, 16, 14);
		tessellateBlockInWorld(tile, tilePos);

		TextureUVCoordinateSet hopper{0, 0, 1, 1};

		const float coords[8]{
			hopper.getInterpolatedU(0.375f), hopper.getInterpolatedV(0.125f),
			hopper.getInterpolatedU(0.125f), hopper.getInterpolatedV(0.625f),
			hopper.getInterpolatedU(0.875f), hopper.getInterpolatedV(0.625f),
			hopper.getInterpolatedU(0.625f), hopper.getInterpolatedV(0.125f)
		};

		// South
		tessellator->quad();
		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.375f, coords[0], coords[1], 15);
		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.125f, coords[2], coords[3], 15);
		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.125f, coords[4], coords[5], 15);
		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.375f, coords[6], coords[7], 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.375f, coords[0], coords[1], 15);
//		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.125f, coords[2], coords[3], 15);
//		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.125f, coords[4], coords[5], 15);
//		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.375f, coords[6], coords[7], 15);

		// East
		tessellator->quad();
		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.375f, coords[0], coords[1], 15);
		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.125f, coords[2], coords[3], 15);
		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.875f, coords[4], coords[5], 15);
		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.625f, coords[6], coords[7], 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.375f, coords[0], coords[1], 15);
//		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.125f, coords[2], coords[3], 15);
//		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.875f, coords[4], coords[5], 15);
//		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.625f, coords[6], coords[7], 15);

		// North
		tessellator->quad();
		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.625f, coords[0], coords[1], 15);
		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.875f, coords[2], coords[3], 15);
		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.875f, coords[4], coords[5], 15);
		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.625f, coords[6], coords[7], 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.625f, coords[0], coords[1], 15);
//		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.875f, coords[2], coords[3], 15);
//		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.875f, coords[4], coords[5], 15);
//		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.625f, coords[6], coords[7], 15);

		// West
		tessellator->quad();
		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.625f, coords[0], coords[1], 15);
		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.875f, coords[2], coords[3], 15);
		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.125f, coords[4], coords[5], 15);
		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.375f, coords[6], coords[7], 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.625f, coords[0], coords[1], 15);
//		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.875f, coords[2], coords[3], 15);
//		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.125f, coords[4], coords[5], 15);
//		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.375f, coords[6], coords[7], 15);

//		switch (m_res->cc_block->overlay_direction) {
//		case Direction::South:
//			bounds.set(6, 0, 0, 10, 2, 10);
//			break;
//		case Direction::North:
//			bounds.set(6, 0, 6, 10, 2, 16);
//			break;
//		case Direction::East:
//			bounds.set(6, 0, 6, 16, 2, 10);
//			break;
//		case Direction::West:
//			bounds.set(0, 0, 6, 10, 2, 10);
//			break;
//		default:
			bounds.set(6, 0, 6, 10, 2, 10);
//			break;
//		}
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateItemFrameInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);

		int rotation = 2;
		setRotatedBox(rotation, 2, 2, 15.5, 14, 14, 16);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 3, 13, 15, 14, 14, 15.5);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 2, 2, 15, 13, 3, 15.5);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 13, 2, 15, 14, 13, 15.5);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(rotation, 2, 3, 15, 3, 14, 15.5);
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateFireInWorld(Tile* tile, const TilePos& tilePos) {
		int x = tilePos.x;
		int y = tilePos.y;
		int z = tilePos.z;

		TextureUVCoordinateSet fire{0, 0, 1, 1};

		tessellator->color(0xFF, 0xFF, 0xFF);

		FlammableBit flammable = FlammableBit(~0);

		const float coords[4]{
			fire.getInterpolatedU(0),
			fire.getInterpolatedV(0),
			fire.getInterpolatedU(1),
			fire.getInterpolatedV(1)
		};

		if (flammable && FlammableBit::Up) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0.95, z + 1, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0, y + 0.95, z + 0, coords[2], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 0.65, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0.65, z + 1, coords[0], coords[3], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0.65, z + 1, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 0.65, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0.95, z + 0, coords[2], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 0.95, z + 1, coords[0], coords[1], 15);
		}

		if (flammable && FlammableBit::Down) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0.05, z + 1, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0, y + 0.05, z + 0, coords[2], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 0.35, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0.35, z + 1, coords[0], coords[3], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0.35, z + 1, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 0.35, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0.05, z + 0, coords[2], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 0.05, z + 1, coords[0], coords[1], 15);
		}

		if (flammable && FlammableBit::North) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 1, y + 0, z + 0.95, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 1, z + 0.60, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 1, z + 0.60, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 0, z + 0.95, coords[2], coords[1], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 1, y + 0, z + 0.85, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 1, z + 0.75, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 1, z + 0.75, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 0, z + 0.85, coords[2], coords[1], 15);
		}

		if (flammable && FlammableBit::South) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0, z + 0.05, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0, y + 1, z + 0.60, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 1, z + 0.40, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0, z + 0.05, coords[2], coords[1], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0, z + 0.15, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0, y + 1, z + 0.25, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 1, z + 0.25, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0, z + 0.15, coords[2], coords[1], 15);
		}

		if (flammable && FlammableBit::East) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0.95, y + 0, z + 0, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0.60, y + 1, z + 0, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0.60, y + 1, z + 1, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0.95, y + 0, z + 1, coords[2], coords[1], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0.85, y + 0, z + 0, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0.75, y + 1, z + 0, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0.75, y + 1, z + 1, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0.85, y + 0, z + 1, coords[2], coords[1], 15);
		}

		if (flammable && FlammableBit::West) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0.05, y + 0, z + 1, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0.40, y + 1, z + 1, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0.40, y + 1, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0.05, y + 0, z + 0, coords[2], coords[1], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0.15, y + 0, z + 1, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0.25, y + 1, z + 1, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0.25, y + 1, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0.15, y + 0, z + 0, coords[2], coords[1], 15);
		}
	}

	inline void tessellateGlassPaneInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);

		enum GlassPaneConnection {
			West = 1,
			North = 2,
			East = 4,
			South = 8,

			EW = East | West,
			SN = South | North
		};

		int directions = GlassPaneConnection::SN | GlassPaneConnection::West;// /*m_res->cc_block->additional_data*/0 & 0xF;
		if (!directions) directions = 15;

		if ((directions & GlassPaneConnection::EW) == GlassPaneConnection::EW) {
			directions ^= GlassPaneConnection::EW;
			bounds.set(0, 0, 0.45f, 1, 0.9999f, 0.55f);
			tessellateBlockInWorld(tile, tilePos);
		}
		if ((directions & GlassPaneConnection::SN) == GlassPaneConnection::SN) {
			directions ^= 0x0A;
			bounds.set(0.45f, 0, 0, 0.55f, 0.9999f, 1);
			tessellateBlockInWorld(tile, tilePos);
		}

		// west
		if (directions & GlassPaneConnection::West) {
			bounds.set(0, 0, 0.45f, 0.5f, 0.9999f, 0.55f);
			tessellateBlockInWorld(tile, tilePos);
		}

		if (directions & GlassPaneConnection::North) {
			bounds.set(0.45f, 0, 0.5f, 0.55f, 0.9999f, 1);
			tessellateBlockInWorld(tile, tilePos);
		}

		if (directions & GlassPaneConnection::East) {
			bounds.set(0.5f, 0, 0.45f, 1, 0.9999f, 0.55f);
			tessellateBlockInWorld(tile, tilePos);
		}

		if (directions & GlassPaneConnection::South) {
			bounds.set(0.45f, 0, 0, 0.55f, 0.9999f, 0.5f);
			tessellateBlockInWorld(tile, tilePos);
		}
	}

	inline void tessellateSignInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);
//		tessellator->setTile(getTile(mb_cfg, texture));

		bounds.set(0, 0.4, 0.45, 1, 1, 0.55);
		tessellateBlockInWorld(tile, tilePos);

		bounds.set(0.45, 0, 0.45, 0.55, 0.4, 0.55);
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateWallSignInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);
//		tessellator->setTile(getTile(mb_cfg, texture));

		setRotatedBounds(2, 0, 0.3, 0.9, 1, 0.9, 1);
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateSlabInWorld(Tile* tile, const TilePos& tilePos) {
		static constexpr AABB visualShape[] {
			{ 0, 0, 0, 1, 0.5, 1 },
			{ 0, 0.5, 0, 1, 1, 1 }
		};

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setTile(getTile(mb_cfg, texture));
//		tessellator->setMaterial(material);

		bounds = visualShape[/*m_res->cc_block->additional_data*/0];
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateVerticalSlabInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setTile(getTile(mb_cfg, texture));
//		tessellator->setMaterial(material);

		bounds.set(0, 0, 0.5, 1, 1, 1);
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateMiddleSlabInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setTile(getTile(mb_cfg, texture));
//		tessellator->setMaterial(material);

   		bounds.set(0, 0.25, 0, 1, 0.75, 1);
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateTrapDoorInWorld(Tile* tile, const TilePos& tilePos) {
		static constexpr AABB visualShapes[]{
			{ 0, 0.8125, 0, 1, 1, 1 },
			{ 0, 0, 0.8125, 1, 1, 1 },
			{ 0, 0, 0, 1, 0.1875, 1 },
			{ 0, 0, 0.8125, 1, 1, 1 }
		};

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);
//		tessellator->setTile(getTile(mb_cfg, texture));

		setRotatedBounds(2, visualShapes[0]);
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateSlopeInWorld(int data, Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);

		bounds.set(0, 0, 0, 1, 1, 1);

        Vector3 p1 { bounds.minX, bounds.minY, bounds.minZ };
	 	Vector3 p2 { bounds.minX, bounds.minY, bounds.maxZ };
        Vector3 p3 { bounds.maxX, bounds.minY, bounds.maxZ };
        Vector3 p4 { bounds.maxX, bounds.minY, bounds.minZ };
		Vector3 p5 { bounds.minX, bounds.maxY, bounds.minZ };
        Vector3 p6 { bounds.minX, bounds.maxY, bounds.maxZ };
        Vector3 p7 { bounds.maxX, bounds.maxY, bounds.maxZ };
        Vector3 p8 { bounds.maxX, bounds.maxY, bounds.minZ };

		bool flip0 = false;
		bool flip1 = false;
		bool flip2 = false;
		bool flip3 = false;
		bool flip4 = false;
		bool flip5 = false;

//		int data = 0;
        switch (data) {
		case 0:
			flip5 = true;
			p5[1] = 0;
			p8[1] = 0;
			break;
		case 1:
			flip2 = true;
			p5[1] = 0;
			p6[1] = 0;
			break;
		case 2:
			flip4 = true;
			p6[1] = 0;
			p7[1] = 0;
			break;
		case 3:
			flip3 = true;
			p7[1] = 0;
			p8[1] = 0;
			break;
		case 4:
			flip4 = true;
			p1[1] = 1;
			p4[1] = 1;
			break;
		case 5:
			flip3 = true;
			p1[1] = 1;
			p2[1] = 1;
			break;
		case 6:
			flip5 = true;
			p2[1] = 1;
			p3[1] = 1;
			break;
		case 7:
			flip2 = true;
			p3[1] = 1;
			p4[1] = 1;
			break;
		case 8:
			flip0 = true;
			p1[2] = 1;
			p5[2] = 1;
			break;
		case 9:
			flip1 = true;
			p2[0] = 1;
			p6[0] = 1;
			break;
		case 10:
			flip0 = true;
			p3[2] = 0;
			p7[2] = 0;
			break;
		case 11:
			flip1 = true;
			p4[0] = 0;
			p8[0] = 0;
			break;
		}

		tessellator->normal(0, 1, 0);
		tessellator->quad(0, 1, flip0 ? 3 : 2, flip0 ? 3 : 0, flip0 ? 1 : 2, flip0 ? 2 : 3);
		rotateFace(tilePos, p5, p6, p7, p8, 0, rotTop, tile->getTexture(0, 0),
				bounds.minX,
				bounds.minZ,
				bounds.maxX,
				bounds.maxZ
		);

		tessellator->normal(0, -1, 0);
		tessellator->quad(0, 1, flip1 ? 3 : 2, flip1 ? 3 : 0, flip1 ? 1 : 2, flip1 ? 2 : 3);
		rotateFace(tilePos, p2, p1, p4, p3, 0, rotBottom, tile->getTexture(1, 0),
				bounds.minX,
				1 - bounds.maxZ,
				bounds.maxX,
				1 - bounds.minZ
		);

		tessellator->normal(0, 0, 1);
		tessellator->quad(0, 1, flip2 ? 3 : 2, flip2 ? 3 : 0, flip2 ? 1 : 2, flip2 ? 2 : 3);
		rotateFace(tilePos, p3, p7, p6, p2, 0, rotNorth, tile->getTexture(2, 0),
				1 - bounds.maxX,
				bounds.minY,
				1 - bounds.minX,
				bounds.maxY
		);

		tessellator->normal(0, 0, -1);
		tessellator->quad(0, 1, flip3 ? 3 : 2, flip3 ? 3 : 0, flip3 ? 1 : 2, flip3 ? 2 : 3);
		rotateFace(tilePos, p1, p5, p8, p4, 0, rotSouth, tile->getTexture(3, 0),
				bounds.minX,
				bounds.minY,
				bounds.maxX,
				bounds.maxY
		);

		tessellator->normal(1, 0, 0);
		tessellator->quad(0, 1, flip4 ? 3 : 2, flip4 ? 3 : 0, flip4 ? 1 : 2, flip4 ? 2 : 3);
		rotateFace(tilePos, p4, p8, p7, p3, 0, rotEast, tile->getTexture(4, 0),
				bounds.minZ,
				bounds.minY,
				bounds.maxZ,
				bounds.maxY
		);

		tessellator->normal(-1, 0, 0);
		tessellator->quad(0, 1, flip5 ? 3 : 2, flip5 ? 3 : 0, flip5 ? 1 : 2, flip5 ? 2 : 3);
		rotateFace(tilePos, p2, p6, p5, p1, 0, rotWest, tile->getTexture(5, 0),
				1 - bounds.maxZ,
				bounds.minY,
				1 - bounds.minZ,
				bounds.maxY
		);
	}

	inline void tessellateCakeInWorld(Tile* tile, const TilePos& tilePos) {
        setRenderBox(1, 0, 1, 15/* - decor*/, 10, 15);
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateCocoaInWorld(Tile* tile, const TilePos& tilePos) {
		int data = 0;

		int rot = data & 3;
		int age = (data >> 2) & 2;

		tessellator->color(0xFF, 0xFF, 0xFF);

		switch (age) {
		case 0:
			setRenderBox(6, 6.5, 10.75, 10, 12, 15);
			break;
		case 1:
			setRenderBox(5.5, 4, 9, 10.5, 12, 15);
			break;
		case 2:
			setRenderBox(5, 0.75, 7, 11, 12, 15);
			break;
		}
		tessellateBlockInWorld(tile, tilePos);

		switch (age) {
		case 0:
			setRenderBox(8, 12, 13, 8, 16, 16);
			break;
		case 1:
			setRenderBox(8, 12, 12.5, 8, 16, 16);
			break;
		case 2:
			setRenderBox(8, 12, 9.75, 8, 16, 16);
			break;
		}
		tessellateBlockInWorld(tile, tilePos);
	}

	inline void tessellateCampfireInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);

		setRenderBox(3, 0, 3, 16 - 3, 1, 16 - 3);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(2, 3, 0, 0, 16, 4, 3);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(4, 3, 0, 0, 16, 4, 3);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(3, 3, 0, 0, 16, 4, 3);
		tessellateBlockInWorld(tile, tilePos);

		setRotatedBox(5, 3, 0, 0, 16, 4, 3);
		tessellateBlockInWorld(tile, tilePos);

		ModelTransform transform0 {
			.origin = {0.5f, 0.0f, 0},
			.rotX = glm::radians(15.0f)
		};
		setRotatedBox(2, 8 - 1.5, 1.5, 2, 8 + 1.5, 1.5 + 14, 2 + 2);
		tessellateAngledInWorld(tile, tilePos, transform0);

		ModelTransform transform1 {
			.origin = {0.5f, 0.0f, 1},
			.rotX = glm::radians(-15.0f)
		};
		setRotatedBox(3, 8 - 1.5, 1.5, 2, 8 + 1.5, 1.5 + 14, 2 + 2);
		tessellateAngledInWorld(tile, tilePos, transform1);

		ModelTransform transform2 {
			.origin = {1.0f, 0.0f, 0.5f},
			.rotZ = glm::radians(9.5f)
		};
		setRotatedBox(4, 8 - 1.5, 1.5, 2, 8 + 1.5, 1.5 + 14, 2 + 2);
		tessellateAngledInWorld(tile, tilePos, transform2);

		ModelTransform transform3 {
			.origin = {0.0f, 0.0f, 0.5f},
			.rotZ = glm::radians(-9.5f)
		};
		setRotatedBox(5, 8 - 1.5, 1.5, 2, 8 + 1.5, 1.5 + 14, 2 + 2);
		tessellateAngledInWorld(tile, tilePos, transform3);
	}

	inline void tessellateStairsInWorld(Tile* tile, const TilePos& tilePos) {
		tessellator->color(0xFF, 0xFF, 0xFF);

		auto stairs = tile->getTexture(0, 0);

		setRenderBox(0, 0, 0, 16, 8, 16);

		tessellator->quad();
		_renderDown(tilePos, stairs);
		tessellator->quad();
		_renderNorth(tilePos, stairs);
		tessellator->quad();
		_renderSouth(tilePos, stairs);
		tessellator->quad();
		_renderEast(tilePos, stairs);
		tessellator->quad();
		_renderWest(tilePos, stairs);

		int data = 5;

		switch (data) {
		case 0:
			setRenderBox(0, 8, 0, 16, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 1:
			setRenderBox(0, 8, 8, 8, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(8, 8, 8, 16, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);

			setRenderBox(0, 8, 0, 16, 8, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 2:
			setRenderBox(8, 8, 8, 16, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 8, 8, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);

			setRenderBox(0, 8, 0, 16, 8, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 3:
			setRenderBox(8, 8, 0, 16, 16, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 8, 16, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);

			setRenderBox(0, 8, 0, 8, 8, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 4:
			setRenderBox(0, 8, 0, 8, 16, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 8, 16, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);

			setRenderBox(8, 8, 0, 16, 8, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 5:
			setRenderBox(0, 8, 8, 16, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 0, 16, 8, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 6:
			setRenderBox(8, 8, 0, 16, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 0, 8, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 7:
			setRenderBox(0, 8, 0, 16, 16, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 8, 16, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 8:
			setRenderBox(0, 8, 0, 8, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(8, 8, 0, 16, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 9:
			setRenderBox(0, 8, 0, 8, 16, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(8, 8, 8, 16, 16, 8);
			tessellator->quad();
			_renderSouth(tilePos, stairs);

			setRenderBox(0, 8, 8, 16, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(8, 8, 0, 16, 8, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 10:
			setRenderBox(8, 8, 0, 16, 16, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 8, 8, 16, 8);
			tessellator->quad();
			_renderSouth(tilePos, stairs);

			setRenderBox(0, 8, 8, 16, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 0, 8, 8, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 11:
			setRenderBox(8, 8, 8, 16, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 8, 8, 16, 8);
			tessellator->quad();
			_renderNorth(tilePos, stairs);

			setRenderBox(0, 8, 0, 16, 16, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 8, 8, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 12:
			setRenderBox(0, 8, 8, 8, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(8, 8, 8, 16, 16, 8);
			tessellator->quad();
			_renderNorth(tilePos, stairs);

			setRenderBox(0, 8, 0, 16, 16, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(8, 8, 8, 16, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 13:
			setRenderBox(8, 8, 8, 16, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 0, 8, 16, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(0, 8, 8, 8, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);

			setRenderBox(8, 8, 0, 16, 8, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		case 14:
			setRenderBox(0, 8, 8, 8, 16, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(8, 8, 0, 16, 16, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			tessellator->quad();
			_renderNorth(tilePos, stairs);
			tessellator->quad();
			_renderSouth(tilePos, stairs);
			tessellator->quad();
			_renderEast(tilePos, stairs);
			tessellator->quad();
			_renderWest(tilePos, stairs);

			setRenderBox(8, 8, 8, 16, 8, 16);
			tessellator->quad();
			_renderUp(tilePos, stairs);

			setRenderBox(0, 8, 0, 8, 8, 8);
			tessellator->quad();
			_renderUp(tilePos, stairs);
			break;
		}
	}

	inline void tessellateRedstoneInWorld(Tile* tile, const TilePos& tilePos) {
		setRenderBox(10, 0, 0, 16, 0, 16);

		tessellator->quadInv();
		_renderDown(tilePos, tile->getTexture(2, 0));

		tessellator->quadInv();
		_renderNorth(tilePos, tile->getTexture(2, 0));

		tessellator->quadInv();
		_renderSouth(tilePos, tile->getTexture(3, 0));

		tessellator->quadInv();
		_renderEast(tilePos, tile->getTexture(4, 0));

		tessellator->quadInv();
		_renderWest(tilePos, tile->getTexture(5, 0));
	}
private:
	inline void face(const TilePos& tilePos, const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4, const TextureUVCoordinateSet& texture, float u0, float v0, float u1, float v1) {
		const float coords[8] {
			texture.getInterpolatedU(u0), texture.getInterpolatedV(v0),
			texture.getInterpolatedU(u1), texture.getInterpolatedV(v1),
		};

		int x = tilePos.x;
		int y = tilePos.y;
		int z = tilePos.z;
        tessellator->vertexUV(p1.x + x, p1.y + y, p1.z + z, coords[0], coords[1], 15);
        tessellator->vertexUV(p2.x + x, p2.y + y, p2.z + z, coords[0], coords[3], 15);
        tessellator->vertexUV(p3.x + x, p3.y + y, p3.z + z, coords[2], coords[3], 15);
        tessellator->vertexUV(p4.x + x, p4.y + y, p4.z + z, coords[2], coords[1], 15);
	}

	inline void rotateFace(const TilePos& pos, const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4, int rot, int texRot, const TextureUVCoordinateSet& texture, float u0, float v0, float u1, float v1) {
		Vector3 vertex1 = p1;
		Vector3 vertex2 = p2;
		Vector3 vertex3 = p3;
		Vector3 vertex4 = p4;

		for (int i = 0; i < rot; i++) {
			vertex1 = {vertex1.z, vertex1.y, 1.0f - vertex1.x};
			vertex2 = {vertex2.z, vertex2.y, 1.0f - vertex2.x};
			vertex3 = {vertex3.z, vertex3.y, 1.0f - vertex3.x};
			vertex4 = {vertex4.z, vertex4.y, 1.0f - vertex4.x};
		}

		Vector2 uv00{u0, v0};
		Vector2 uv01{u0, v1};
		Vector2 uv11{u1, v1};
		Vector2 uv10{u1, v0};

		for (int i = 0; i < texRot; i++) {
			uv00 = {uv00.y, 1.0f - uv00.x};
			uv01 = {uv01.y, 1.0f - uv01.x};
			uv11 = {uv11.y, 1.0f - uv11.x};
			uv10 = {uv10.y, 1.0f - uv10.x};
		}

		const float coords[8] {
			texture.getInterpolatedU(uv00.x), texture.getInterpolatedV(uv00.y),
			texture.getInterpolatedU(uv01.x), texture.getInterpolatedV(uv01.y),
			texture.getInterpolatedU(uv11.x), texture.getInterpolatedV(uv11.y),
			texture.getInterpolatedU(uv10.x), texture.getInterpolatedV(uv10.y),
		};

		int x = pos.x;
		int y = pos.y;
		int z = pos.z;
        tessellator->vertexUV(vertex1.x + x, vertex1.y + y, vertex1.z + z, coords[0], coords[1], 15);
        tessellator->vertexUV(vertex2.x + x, vertex2.y + y, vertex2.z + z, coords[2], coords[3], 15);
        tessellator->vertexUV(vertex3.x + x, vertex3.y + y, vertex3.z + z, coords[4], coords[5], 15);
        tessellator->vertexUV(vertex4.x + x, vertex4.y + y, vertex4.z + z, coords[6], coords[7], 15);
	}

	inline void apply(ModelTransform* modelTransform, Vector3& p1, Vector3& p2, Vector3& p3, Vector3& p4) {
		for (auto it = modelTransform; it != nullptr; it = it->next) {
			it->apply(p1, p2, p3, p4);
		}
	}
};
