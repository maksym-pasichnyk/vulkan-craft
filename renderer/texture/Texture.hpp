#pragma once

#include <cstdint>

struct RenderTexture;
struct TextureManager;

struct Texture {
	RenderTexture* renderTexture{nullptr};

	Texture() = default;
	Texture(RenderTexture* renderTexture)
		: renderTexture(renderTexture) {}

	virtual ~Texture() = default;
	virtual void loadTexture(TextureManager* textureManager) {}
};