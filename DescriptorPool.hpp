#pragma once

#include "RenderSystem.hpp"

namespace vkx {
	inline vk::DescriptorPool createDescriptorPool(uint32_t maxSets, span<const vk::DescriptorPoolSize> poolSizes) {
		vk::DescriptorPoolCreateInfo createInfo;
		createInfo.maxSets = maxSets;
		createInfo.poolSizeCount = std::size(poolSizes);
		createInfo.pPoolSizes = std::data(poolSizes);

		return RenderSystem::Get()->device().createDescriptorPool(createInfo, nullptr);
	}

	inline vk::DescriptorSet allocate(vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout setLayout) {
		vk::DescriptorSetAllocateInfo allocateInfo;
		allocateInfo.descriptorPool = descriptorPool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &setLayout;

		vk::DescriptorSet descriptorSet;
		RenderSystem::Get()->device().allocateDescriptorSets(&allocateInfo, &descriptorSet);
		return descriptorSet;
	}
}