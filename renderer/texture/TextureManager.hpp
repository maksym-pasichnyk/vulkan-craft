#pragma once

#include "renderer/RenderContext.hpp"

#include "Texture.hpp"

#include <set>
#include <map>


struct TextureManager {
	TextureManager(RenderContext* renderContext, ResourceManager* resourceManager)
		: renderContext(renderContext)
		, resourceManager(resourceManager) {}

	Texture* getTexture(const std::string& name) {
		auto it = textures.find(name);
		if (it != textures.end()) {
			return it->second;
		}
		auto texture = loadTexture(name);
		texture->loadTexture(this);
		textures.emplace(name, texture);
		return texture;
	}

	void upload(std::string name, Texture* texture) {
		texture->loadTexture(this);
		textures.insert_or_assign(std::move(name), texture);
	}

private:
	Texture* loadTexture(const std::string& name) {
		if (auto data = resourceManager->loadTextureData(name)) {
			auto renderTexture = createTexture(vk::Format::eR8G8B8A8Unorm, data->width, data->height, data->channels, data->pixels);
			stbi_image_free(data->pixels);
			return new Texture(renderTexture);
		}
		return nullptr;
	}

public:
	RenderTexture* createTexture(vk::Format format, uint32_t width, uint32_t height, int channels, const void* pixels) {
		auto texture = renderContext->createTexture2D(format, width, height);
		renderContext->textureSubImage2D(texture, width, height, channels, pixels);
		return texture;
	}

private:
	std::map<std::string, Texture*> textures;

	RenderSystem* core = RenderSystem::Instance();

	RenderContext* renderContext;
	ResourceManager* resourceManager;
};