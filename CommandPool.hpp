#pragma once

#include "renderer/RenderSystem.hpp"

struct CommandPool {
	vk::CommandPool _commandPool;

	inline static CommandPool create(uint32_t queueFamilyIndex, vk::CommandPoolCreateFlags flags) {
		vk::CommandPoolCreateInfo createInfo;
		createInfo.flags = flags;
		createInfo.queueFamilyIndex = queueFamilyIndex;

		return {RenderSystem::Instance()->device().createCommandPool(createInfo, nullptr)};
	}

	void destroy() {
		RenderSystem::Instance()->device().destroyCommandPool(_commandPool, nullptr);
	}

	inline vk::CommandBuffer allocate(vk::CommandBufferLevel level) {
		vk::CommandBufferAllocateInfo allocateInfo {
			.commandPool = _commandPool,
			.level = level,
			.commandBufferCount = 1
		};

		vk::CommandBuffer commandBuffer;
		RenderSystem::Instance()->device().allocateCommandBuffers(&allocateInfo, &commandBuffer);
		return commandBuffer;
	}

	void free(vk::CommandBuffer commandBuffer) {
		RenderSystem::Instance()->device().freeCommandBuffers(_commandPool, 1, &commandBuffer);
	}
};