#pragma once

#include <cstdint>

#include <vulkan/vulkan.hpp>

#include "RenderSystem.hpp"

#include "Image.hpp"
#include "Buffer.hpp"

struct Texture {
	Image image;
	vk::ImageView view;
	vk::DescriptorSet descriptor;
	vk::Sampler sampler{nullptr};

	inline static Texture create2D(vk::Format format, uint32_t width, uint32_t height, vk::Sampler sampler = nullptr) {
		vk::ImageCreateInfo ImageCI{
				.imageType = vk::ImageType::e2D,
				.format = format,
				.extent = {
						.width = width,
						.height = height,
						.depth = 1
				},
				.mipLevels = 1,
				.arrayLayers = 1,
				.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
		};
		auto image = Image::create(ImageCI, {.usage = VMA_MEMORY_USAGE_GPU_ONLY});
		auto view = RenderSystem::Get()->device().createImageView({
			.image = image,
			.viewType = vk::ImageViewType::e2D,
			.format = format,
			.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
		});

		return { .image = image, .view = view, .sampler = sampler };
	}

	void updateDescriptorSet(vk::DescriptorSet dstSet) {
		descriptor = dstSet;

		vk::DescriptorImageInfo imageInfo{
				.sampler = sampler,
				.imageView = view,
				.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
		};

		vk::WriteDescriptorSet writeDescriptorSet{
				.dstSet = dstSet,
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &imageInfo,
		};

		RenderSystem::Get()->device().updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	}

	void destroy() {
		RenderSystem::Get()->device().destroyImageView(view, nullptr);
		image.destroy();
	}
};