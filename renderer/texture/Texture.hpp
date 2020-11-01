#pragma once

#include <cstdint>

#include <vulkan/vulkan.hpp>

#include "RenderSystem.hpp"

#include "Image.hpp"
#include "Buffer.hpp"

struct Texture {
	Image image;
	vk::ImageView view;

	void destroy() {
		RenderSystem::Instance()->device().destroyImageView(view, nullptr);
		image.destroy();
	}
};