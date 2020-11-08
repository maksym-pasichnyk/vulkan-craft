#pragma once

#include "resources/ResourceManager.hpp"

#include "client/util/Handle.hpp"

struct Colormap {
	static void initColormaps(Handle<ResourceManager> resourceManager) {
		birch = resourceManager->loadTextureData("textures/colormap/birch").value();
		evergreen = resourceManager->loadTextureData("textures/colormap/evergreen").value();
		foliage = resourceManager->loadTextureData("textures/colormap/foliage").value();
		grass = resourceManager->loadTextureData("textures/colormap/grass").value();
		swamp_foliage = resourceManager->loadTextureData("textures/colormap/swamp_foliage").value();
		swamp_grass = resourceManager->loadTextureData("textures/colormap/swamp_grass").value();
	}
private:
	inline static constinit NativeImage birch;
	inline static constinit NativeImage evergreen;
	inline static constinit NativeImage foliage;
	inline static constinit NativeImage grass;
	inline static constinit NativeImage swamp_foliage;
	inline static constinit NativeImage swamp_grass;
};