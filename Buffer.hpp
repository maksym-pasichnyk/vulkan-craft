#pragma once

#include "RenderSystem.hpp"

struct Buffer {
	Buffer() = default;

	inline static Buffer create(const vk::BufferCreateInfo& BufferCI, const VmaAllocationCreateInfo &AllocCI) {
		VkBuffer buffer;
		VmaAllocation allocation;

		VkBufferCreateInfo vkBufferCI = BufferCI;
		vmaCreateBuffer(RenderSystem::Instance()->allocator(), &vkBufferCI, &AllocCI, &buffer, &allocation, nullptr);
		return { buffer, allocation };
	}

	inline void destroy() {
		vmaDestroyBuffer(RenderSystem::Instance()->allocator(), buffer, allocation);
	}

	void* map() {
		void *pData;
		vmaMapMemory(RenderSystem::Instance()->allocator(), allocation, &pData);
		return pData;
	}

	void unmap() {
		vmaUnmapMemory(RenderSystem::Instance()->allocator(), allocation);
	}

	operator vk::Buffer() {
		return buffer;
	}
	operator VkBuffer() {
		return buffer;
	}

private:
	Buffer(VkBuffer buffer, VmaAllocation allocation) : buffer(buffer), allocation(allocation) {}

	VkBuffer buffer{nullptr};
	VmaAllocation allocation{nullptr};
};