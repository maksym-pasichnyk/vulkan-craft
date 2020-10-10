#pragma once

#include <vulkan/vulkan.hpp>

#include "Buffer.hpp"

struct Image {
	VkImage image{nullptr};
	VmaAllocation allocation{nullptr};

	vk::Extent3D extent;

	inline static Image create(const vk::ImageCreateInfo &ImageCI, const VmaAllocationCreateInfo &AllocCI) {
		VkImage image;
		VmaAllocation allocation;

		VkImageCreateInfo vkImageCI = ImageCI;
		vmaCreateImage(RenderSystem::Get()->allocator(), &vkImageCI, &AllocCI, &image, &allocation, nullptr);

		return {
			.image = image,
			.allocation = allocation,
			.extent = ImageCI.extent
		};
	}

	operator vk::Image() {
		return image;
	}

	void destroy() {
		vmaDestroyImage(RenderSystem::Get()->allocator(), image, allocation);
	}

//	void setTextureData(const void *pixels, vk::DeviceSize size) {
//		vk::BufferCreateInfo BufferCI{ .size = size, .usage = vk::BufferUsageFlagBits::eTransferSrc };
//
//		auto srcBuffer = Buffer::create(BufferCI, { .usage = VMA_MEMORY_USAGE_CPU_ONLY });
//		memcpy(srcBuffer.map(), pixels, size);
//		srcBuffer.unmap();
//
//		auto command_buffer = vkx::allocate(_commandPool, vk::CommandBufferLevel::ePrimary);
//		command_buffer.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
//
//		auto cmd = [&] {
//			vk::CommandBufferAllocateInfo CmdBufferAI {
//				.commandPool = Enigma::Get()->commandPool(),
//				.level = vk::CommandBufferLevel::ePrimary,
//				.commandBufferCount = 1
//			};
//
//			vk::CommandBuffer commandBuffer;
//			Enigma::Get()->device().allocateCommandBuffers(&CmdBufferAI, &commandBuffer);
//			return commandBuffer;
//		}();
//
//		cmd.begin(vk::CommandBufferBeginInfo{
//			.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
//		});
//
//		vk::ImageMemoryBarrier copyBarrier{
//			.dstAccessMask = vk::AccessFlagBits::eTransferWrite,
//			.oldLayout = vk::ImageLayout::eUndefined,
//			.newLayout = vk::ImageLayout::eTransferDstOptimal,
//			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//			.image = image,
//			.subresourceRange {
//				.aspectMask = vk::ImageAspectFlagBits::eColor,
//				.levelCount = 1,
//				.layerCount = 1
//			}
//		};
//
//		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &copyBarrier);
//
//		vk::BufferImageCopy region {
//			.imageSubresource {
//				.aspectMask = vk::ImageAspectFlagBits::eColor,
//				.layerCount = 1
//			},
//			.imageExtent = extent
//		};
//		cmd.copyBufferToImage(srcBuffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
//
//		vk::ImageMemoryBarrier writeBarrier {
//			.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
//			.dstAccessMask = vk::AccessFlagBits::eShaderRead,
//			.oldLayout = vk::ImageLayout::eTransferDstOptimal,
//			.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
//			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//			.image = image,
//			.subresourceRange = {
//				.aspectMask = vk::ImageAspectFlagBits::eColor,
//				.levelCount = 1,
//				.layerCount = 1,
//			}
//		};
//		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &writeBarrier);
//		cmd.end();
//
//		vk::SubmitInfo submitInfo;
//		submitInfo.commandBufferCount = 1;
//		submitInfo.pCommandBuffers = &cmd;
//
//		Enigma::Get()->graphicsQueue().submit(1, &submitInfo, nullptr);
//		Enigma::Get()->graphicsQueue().waitIdle();
//
//		srcBuffer.destroy();
//
//		Enigma::Get()->device().freeCommandBuffers(Enigma::Get()->commandPool(), 1, &cmd);
//	}
};
