#pragma once

#include "RenderSystem.hpp"
#include "Buffer.hpp"

struct RenderBuffer {
	Buffer VertexBuffer{};
	Buffer IndexBuffer{};

	int VertexCount;
	int IndexCount;

	vk::DeviceSize VertexBufferSize{0};
	vk::DeviceSize IndexBufferSize{0};

	void SetVertexBufferCount(int count, size_t elementSize) {
		VertexBuffer.destroy();

		VertexCount = count;
		VertexBufferSize = count * elementSize;
		if (VertexBufferSize > 0) {
			vk::BufferCreateInfo BufferCI {
				.size = VertexBufferSize,
				.usage = vk::BufferUsageFlagBits::eVertexBuffer
			};
			VertexBuffer = Buffer::create(BufferCI, {.usage = VMA_MEMORY_USAGE_CPU_ONLY});
		}
	}

	void SetVertexBufferData(const void* data, int dataStart, int meshBufferStart, int size) {
		auto dst_ptr = VertexBuffer.map();
		std::memcpy((std::byte*)dst_ptr + meshBufferStart, (std::byte*)data + dataStart, size);
		VertexBuffer.unmap();
	}

	void SetIndexBufferCount(int count, size_t elementSize) {
		IndexBuffer.destroy();

		IndexCount = count;
		IndexBufferSize = count * elementSize;
		if (IndexBufferSize > 0) {
			vk::BufferCreateInfo BufferCI {
				.size = IndexBufferSize,
				.usage = vk::BufferUsageFlagBits::eIndexBuffer
			};
			IndexBuffer = Buffer::create(BufferCI, {.usage = VMA_MEMORY_USAGE_CPU_ONLY});
		}
	}

	void SetIndexBufferData(const void* data, int dataStart, int meshBufferStart, int size) {
		auto dst_ptr = IndexBuffer.map();
		std::memcpy((std::byte*)dst_ptr + meshBufferStart, (std::byte*)data + dataStart, size);
		IndexBuffer.unmap();
	}

	void destroy() {
		VertexBuffer.destroy();
		IndexBuffer.destroy();
	}
};