#pragma once

#include "RenderSystem.hpp"
#include "Buffer.hpp"

struct RenderBuffer {
	Buffer VertexBuffer{};
	Buffer IndexBuffer{};

	vk::DeviceSize VertexBufferSize{0};
	vk::DeviceSize IndexBufferSize{0};

	void SetVertexBufferSize(int buffer_size) {
		VertexBuffer.destroy();

		VertexBufferSize = buffer_size;
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

	void SetIndexBufferSize(int buffer_size) {
		IndexBuffer.destroy();

		IndexBufferSize = buffer_size;
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