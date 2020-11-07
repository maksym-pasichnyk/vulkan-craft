#pragma once

#include "VertexBuilder.hpp"

#include "client/renderer/RenderBuffer.hpp"
#include "client/renderer/model/ModelFormat.hpp"
#include "client/renderer/material/Material.hpp"
#include "client/renderer/TexturedQuad.hpp"

#include "client/util/Handle.hpp"

struct EntityRenderer {
	Handle<Material> material;

	EntityRenderer(Handle<Material> material, const ModelFormat& model_format) : material(material) {
		auto texture_width = model_format.texture_width;
		auto texture_height = model_format.texture_height;

		VertexBuilder vertexBuilder;
		for (auto& bone : model_format.bones) {
			if (bone.neverRender) continue;

			for (auto& cube : bone.cubes) {
				const float posX = cube.origin.x;
				const float posY = cube.origin.y;
				const float posZ = cube.origin.z;

				const float sizeX = cube.size.x;
				const float sizeY = cube.size.y;
				const float sizeZ = cube.size.z;

				const auto [u, v] = std::get<Vector2>(cube.uv);

				const float x0 = posX / 16.0f;
				const float y0 = posY / 16.0f;
				const float z0 = posZ / 16.0f;

				const float x1 = (posX + sizeX) / 16.0f;
				const float y1 = (posY + sizeY) / 16.0f;
				const float z1 = (posZ + sizeZ) / 16.0f;

				const float u0 = (u) / texture_width;
				const float u1 = (u + sizeZ) / texture_width;
				const float u2 = (u + sizeZ + sizeX) / texture_width;
				const float u3 = (u + sizeZ + sizeX + sizeX) / texture_width;
				const float u4 = (u + sizeZ + sizeX + sizeZ) / texture_width;
				const float u5 = (u + sizeZ + sizeX + sizeZ + sizeX) / texture_width;

				const float v0 = (v) / texture_height;
				const float v1 = (v + sizeZ) / texture_height;
				const float v2 = (v + sizeZ + sizeY) / texture_height;

				TexturedQuad quad0{
					.vertices{
						PositionTextureVertex{x0, y0, z0, u1, v2},
						PositionTextureVertex{x0, y1, z0, u1, v1},
						PositionTextureVertex{x1, y1, z0, u2, v1},
						PositionTextureVertex{x1, y0, z0, u2, v2}
					},
					.normal{0, 0, -1.0f}
				};
				TexturedQuad quad1{
					.vertices{
						PositionTextureVertex{x1, y0, z0, u1, v2},
						PositionTextureVertex{x1, y1, z0, u1, v1},
						PositionTextureVertex{x1, y1, z1, u0, v1},
						PositionTextureVertex{x1, y0, z1, u0, v2}
					},
					.normal{1.0f, 0, 0}
				};
				TexturedQuad quad2{
					.vertices{
						PositionTextureVertex{x1, y0, z1, u4, v2},
						PositionTextureVertex{x1, y1, z1, u4, v1},
						PositionTextureVertex{x0, y1, z1, u5, v1},
						PositionTextureVertex{x0, y0, z1, u5, v2}
					},
					.normal{0, 0, 1.0f}
				};
				TexturedQuad quad3{
					.vertices{
						PositionTextureVertex{x0, y0, z1, u0, v2},
						PositionTextureVertex{x0, y1, z1, u0, v1},
						PositionTextureVertex{x0, y1, z0, u1, v1},
						PositionTextureVertex{x0, y0, z0, u1, v2}
					},
					.normal{-1.0f, 0, 0}
				};
				TexturedQuad quad4{
					.vertices{
						PositionTextureVertex{x0, y1, z0, u1, v1},
						PositionTextureVertex{x0, y1, z1, u1, v0},
						PositionTextureVertex{x1, y1, z1, u2, v0},
						PositionTextureVertex{x1, y1, z0, u2, v1}
					},
					.normal{0, 1.0f, 0}
				};
				TexturedQuad quad5{
					.vertices{
						PositionTextureVertex{x0, y0, z1, u2, v1},
						PositionTextureVertex{x0, y0, z0, u2, v0},
						PositionTextureVertex{x1, y0, z0, u3, v0},
						PositionTextureVertex{x1, y0, z1, u3, v1}
					},
					.normal{0, -1.0f, 0}
				};

//				ModelBox modelBox{
//					.quads {
//						quad0,
//						quad1,
//						quad2,
//						quad3,
//						quad4,
//						quad5
//					}
//				};

				std::array quads {
					quad0,
					quad1,
					quad2,
					quad3,
					quad4,
					quad5
				};

				for (auto& quad : quads) {
					auto normal = quad.normal;

					vertexBuilder.addQuad(0, 1, 2, 0, 2, 3);
					for (auto &&vertex : quad.vertices) {
						vertexBuilder.vertices.emplace_back(vertex.x, vertex.y, vertex.z, vertex.u, vertex.v, normal.x, normal.y, normal.z);
					}
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
	RenderBuffer renderBuffer;
};
