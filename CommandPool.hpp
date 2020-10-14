#pragma once

#include "RenderSystem.hpp"

struct CommandPool {
	vk::CommandPool _commandPool;

	inline static CommandPool create(uint32_t queue_index, vk::CommandPoolCreateFlags flags) {
		vk::CommandPoolCreateInfo createInfo;
		createInfo.flags = flags;
		createInfo.queueFamilyIndex = queue_index;
		return {RenderSystem::Get()->device().createCommandPool(createInfo, nullptr)};
	}

	void destroy() {
		RenderSystem::Get()->device().destroyCommandPool(_commandPool, nullptr);
	}

	inline vk::CommandBuffer allocate(vk::CommandBufferLevel level) {
		vk::CommandBufferAllocateInfo allocateInfo {
			.commandPool = _commandPool,
			.level = level,
			.commandBufferCount = 1
		};

		vk::CommandBuffer commandBuffer;
		RenderSystem::Get()->device().allocateCommandBuffers(&allocateInfo, &commandBuffer);
		return commandBuffer;
	}

	void free(vk::CommandBuffer commandBuffer) {
		RenderSystem::Get()->device().freeCommandBuffers(_commandPool, 1, &commandBuffer);
	}
};