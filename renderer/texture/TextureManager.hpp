#pragma once

#include "renderer/RenderContext.hpp"

#include "Texture.hpp"

#include "stb_image.hpp"

#include <map>

struct TextureManager {
	TextureManager(RenderContext* renderContext, ResourceManager* resourceManager)
	: renderContext(renderContext)
	, resourceManager(resourceManager) {}

	Texture* getTexture(const std::string& name) {
		auto& texture = textures[name];
		if (texture == nullptr) {
			texture = loadTexture(name);
		}
		return texture;
	}

	Texture* loadTexture(const std::string& name) {
		std::vector<char> bytes;
		if (auto bytes = resourceManager->loadFile(name)) {
			auto data = reinterpret_cast<const unsigned char *>(bytes->data());

			int width, height, channels;

			auto pixels = stbi_load_from_memory(data, bytes->size(), &width, &height, &channels, 0);

			auto texture = createTexture(vk::Format::eR8G8B8A8Unorm, width, height);

			setTextureData(texture, width, height, channels, pixels);
			return texture;
		}
		return nullptr;
	}

private:
	Texture* createTexture(vk::Format format, uint32_t width, uint32_t height) {
		vk::ImageCreateInfo imageCI{
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
		auto image = Image::create(imageCI, {.usage = VMA_MEMORY_USAGE_GPU_ONLY});

		vk::ImageViewCreateInfo imageViewCI{
			.image = image,
			.viewType = vk::ImageViewType::e2D,
			.format = format,
			.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
		};
		auto view = RenderSystem::Instance()->device().createImageView(imageViewCI);

		return new Texture{
			.image = image,
			.view = view
		};
	}

	void setTextureData(Texture* texture, uint32_t width, uint32_t height, int channels, const void* pixels) {
		auto cmd = renderContext->commandPool.allocate(vk::CommandBufferLevel::ePrimary);
		cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

		vk::DeviceSize bufferSize = width * height * channels;

		vk::BufferCreateInfo srcBufferCI{.size = bufferSize, .usage = vk::BufferUsageFlagBits::eTransferSrc};

		auto srcBuffer = Buffer::create(srcBufferCI, {.usage = VMA_MEMORY_USAGE_CPU_ONLY});
		std::memcpy(srcBuffer.map(), pixels, bufferSize);
		srcBuffer.unmap();

		vk::ImageMemoryBarrier copy_barrier{
				.dstAccessMask = vk::AccessFlagBits::eTransferWrite,
				.oldLayout = vk::ImageLayout::eUndefined,
				.newLayout = vk::ImageLayout::eTransferDstOptimal,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = texture->image,
				.subresourceRange {
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.levelCount = 1,
						.layerCount = 1
				}
		};

		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &copy_barrier);

		vk::BufferImageCopy region{
				.imageSubresource {
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.layerCount = 1
				},
				.imageExtent = {
						.width = static_cast<uint32_t>(width),
						.height = static_cast<uint32_t>(height),
						.depth = 1
				}
		};

		cmd.copyBufferToImage(srcBuffer, texture->image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

		vk::ImageMemoryBarrier use_barrier{
				.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
				.dstAccessMask = vk::AccessFlagBits::eShaderRead,
				.oldLayout = vk::ImageLayout::eTransferDstOptimal,
				.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = texture->image,
				.subresourceRange = {
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.levelCount = 1,
						.layerCount = 1,
				}
		};

		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &use_barrier);
		cmd.end();

		vk::SubmitInfo submitInfo{
				.commandBufferCount = 1,
				.pCommandBuffers = &cmd
		};
		core->graphicsQueue().submit(1, &submitInfo, nullptr);
		core->graphicsQueue().waitIdle();

		renderContext->commandPool.free(cmd);

		srcBuffer.destroy();
	}

private:
	std::map<std::string, Texture*> textures;

	RenderSystem* core = RenderSystem::Instance();

	RenderContext* renderContext;
	ResourceManager* resourceManager;
};