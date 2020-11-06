#pragma once

#include "client/renderer/model/ModelRenderer.hpp"
#include "client/renderer/RenderContext.hpp"

#include "VertexBuilder.hpp"
#include "client/renderer/RenderBuffer.hpp"
#include "client/renderer/texture/Texture.hpp"
#include "client/renderer/texture/TextureManager.hpp"
#include "client/util/stb_image.hpp"

#include "client/util/Json.hpp"

#include "client/renderer/material/Material.hpp"
#include "client/renderer/material/MaterialManager.hpp"

#include "client/AppPlatform.hpp"

#include "client/util/Handle.hpp"

struct EntityRenderer {
	ModelRenderer modelRenderer;

	Handle<Material> material;

	EntityRenderer(Handle<ResourceManager> resourceManager, Handle<Material> material) : material(material) {
		for (auto&& cube : modelRenderer.cubes) {
			for (auto&& quad : cube.quads) {
				if (!quad.has_value()) continue;
				auto normal = quad->normal;

				vertexBuilder.addQuad(0, 1, 2, 0, 2, 3);
				for (auto&& vertex : quad->vertices) {
					vertexBuilder.vertices.emplace_back(
						vertex.x, vertex.y, vertex.z,
						vertex.u, vertex.v,
						normal.x, normal.y, normal.z
					);
				}
			}
		}

		renderBuffer.SetIndexBufferCount(vertexBuilder.indices.size(), sizeof(int));
		renderBuffer.SetVertexBufferCount(vertexBuilder.vertices.size(), sizeof(Vertex));

		renderBuffer.SetIndexBufferData(vertexBuilder.indices.data(), 0, 0, sizeof(int) * vertexBuilder.indices.size());
		renderBuffer.SetVertexBufferData(vertexBuilder.vertices.data(), 0, 0, sizeof(Vertex) * vertexBuilder.vertices.size());
	}

	~EntityRenderer() {
		renderBuffer.destroy();
	}

	void render(vk::CommandBuffer cmd, CameraTransform& transform) {
		vk::DeviceSize offset{0};

		vk::Buffer vertexBuffers[] {
			renderBuffer.VertexBuffer
		};

		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, material->pipeline);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, material->pipelineLayout, 0, 1, &material->descriptorSet, 0, nullptr);
		cmd.pushConstants(material->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(CameraTransform), &transform);

		cmd.bindVertexBuffers(0, 1, vertexBuffers, &offset);
		cmd.bindIndexBuffer(renderBuffer.IndexBuffer, 0, vk::IndexType::eUint32);

		cmd.drawIndexed(renderBuffer.IndexCount, 1, 0, 0, 0);
	}

private:
	RenderSystem* core = RenderSystem::Instance();
	Texture* texture;

	RenderBuffer renderBuffer;
	VertexBuilder vertexBuilder;
};
