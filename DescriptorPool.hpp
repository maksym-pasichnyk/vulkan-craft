#pragma once

#include "renderer/RenderSystem.hpp"


struct DescriptorPool {
	vk::DescriptorPool _descriptorPool;

	inline static DescriptorPool create(uint32_t maxSets, std::span<const vk::DescriptorPoolSize> poolSizes) {
		vk::DescriptorPoolCreateInfo createInfo {
			.maxSets = maxSets,
			.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
			.pPoolSizes = std::data(poolSizes)
		};

		return {RenderSystem::Instance()->device().createDescriptorPool(createInfo, nullptr)};
	}

	inline vk::DescriptorSet allocate(vk::DescriptorSetLayout setLayout) {
		vk::DescriptorSetAllocateInfo allocateInfo {
			.descriptorPool = _descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &setLayout
		};

		vk::DescriptorSet descriptorSet;
		RenderSystem::Instance()->device().allocateDescriptorSets(&allocateInfo, &descriptorSet);
		return descriptorSet;
	}

	inline void free(vk::DescriptorSet descriptorSet) {
		RenderSystem::Instance()->device().freeDescriptorSets(_descriptorPool, 1, &descriptorSet);
	}

	inline void destroy() {
		RenderSystem::Instance()->device().destroyDescriptorPool(_descriptorPool, nullptr);
	}
};