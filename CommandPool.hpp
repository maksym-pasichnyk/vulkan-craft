#pragma once

#include "RenderSystem.hpp"

namespace vkx {
	inline vk::CommandPool createCommandPool(uint32_t queue_index, vk::CommandPoolCreateFlags flags) {
		vk::CommandPoolCreateInfo createInfo;
		createInfo.flags = flags;
		createInfo.queueFamilyIndex = queue_index;
		return RenderSystem::Get()->device().createCommandPool(createInfo, nullptr);
	}

	inline vk::CommandBuffer allocate(vk::CommandPool commandPool, vk::CommandBufferLevel level) {
		vk::CommandBufferAllocateInfo allocateInfo;
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = level;
		allocateInfo.commandBufferCount = 1;

		vk::CommandBuffer commandBuffer;
		RenderSystem::Get()->device().allocateCommandBuffers(&allocateInfo, &commandBuffer);
		return commandBuffer;
	}
}
