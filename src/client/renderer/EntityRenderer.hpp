#pragma once

#include "VertexBuilder.hpp"

#include "client/renderer/RenderBuffer.hpp"
#include "client/renderer/model/ModelFormat.hpp"
#include "client/renderer/material/Material.hpp"
#include "client/renderer/TexturedQuad.hpp"

#include "client/util/Handle.hpp"

struct EntityRenderer {
	Handle<Material> material;

	EntityRenderer(Handle<Material> material, Handle<ModelFormat> model_format) : material(material) {
		auto texture_width = model_format->texture_width;
		auto texture_height = model_format->texture_height;

		VertexBuilder builder;
		for (auto& [name, bone] : model_format->bones) {
			if (bone->neverRender) continue;

			auto origin = bone->pivot;

			for (auto& cube : bone->cubes) {
				auto& faces = cube.faces;
				auto& from = cube.origin;
				auto& size = cube.size;
				auto& uv = cube.uv;

				auto x0 = from.x / 16.0f;
				auto y0 = from.y / 16.0f;
				auto z0 = from.z / 16.0f;

				auto x1 = x0 + size.x / 16.0f;
				auto y1 = y0 + size.y / 16.0f;
				auto z1 = z0 + size.z / 16.0f;

				if (cube.uv_box) {
					auto u = uv.x;
					auto v = uv.y;

					auto u0 = (u) / texture_width;
					auto u1 = (u + size.z) / texture_width;
					auto u2 = (u + size.z + size.x) / texture_width;
					auto u3 = (u + size.z + size.x + size.x) / texture_width;
					auto u4 = (u + size.z + size.x + size.z) / texture_width;
					auto u5 = (u + size.z + size.x + size.z + size.x) / texture_width;

					auto v0 = (v) / texture_height;
					auto v1 = (v + size.z) / texture_height;
					auto v2 = (v + size.z + size.y) / texture_height;

					buildQuad(builder, TexturedQuad{
						.vertices{
							PositionTextureVertex{x0, y0, z0, u1, v2},
							PositionTextureVertex{x0, y1, z0, u1, v1},
							PositionTextureVertex{x1, y1, z0, u2, v1},
							PositionTextureVertex{x1, y0, z0, u2, v2}
						},
						.normal{0, 0, -1.0f}
					});

					buildQuad(builder, TexturedQuad{
						.vertices{
							PositionTextureVertex{x1, y0, z0, u1, v2},
							PositionTextureVertex{x1, y1, z0, u1, v1},
							PositionTextureVertex{x1, y1, z1, u0, v1},
							PositionTextureVertex{x1, y0, z1, u0, v2}
						},
						.normal{1.0f, 0, 0}
					});

					buildQuad(builder, TexturedQuad{
						.vertices{
							PositionTextureVertex{x1, y0, z1, u4, v2},
							PositionTextureVertex{x1, y1, z1, u4, v1},
							PositionTextureVertex{x0, y1, z1, u5, v1},
							PositionTextureVertex{x0, y0, z1, u5, v2}
						},
						.normal{0, 0, 1.0f}
					});

					buildQuad(builder, TexturedQuad{
						.vertices{
							PositionTextureVertex{x0, y0, z1, u0, v2},
							PositionTextureVertex{x0, y1, z1, u0, v1},
							PositionTextureVertex{x0, y1, z0, u1, v1},
							PositionTextureVertex{x0, y0, z0, u1, v2}
						},
						.normal{-1.0f, 0, 0}
					});

					buildQuad(builder, TexturedQuad{
						.vertices{
							PositionTextureVertex{x0, y1, z0, u1, v1},
							PositionTextureVertex{x0, y1, z1, u1, v0},
							PositionTextureVertex{x1, y1, z1, u2, v0},
							PositionTextureVertex{x1, y1, z0, u2, v1}
						},
						.normal{0, 1.0f, 0}
					});

					buildQuad(builder, TexturedQuad{
						.vertices{
							PositionTextureVertex{x0, y0, z1, u2, v0},
							PositionTextureVertex{x0, y0, z0, u2, v1},
							PositionTextureVertex{x1, y0, z0, u3, v1},
							PositionTextureVertex{x1, y0, z1, u3, v0}
						},
						.normal{0, -1.0f, 0}
					});
				} else {
					if (auto face = cube.faces.find("south"); face != cube.faces.end()) {
						auto uv = face->second.uv;
						auto u = uv.x;
						auto v = uv.y;

						buildFace(builder,
							{x0, y0, z0},
							{x0, y1, z0},
							{x1, y1, z0},
							{x1, y0, z0},
							u / float(texture_width),
							v / float(texture_width),
							(u + size.x) / float(texture_width),
							(v + size.y) / float(texture_width),
							{0, 0, -1.0f}
						);
					}

					if (auto face = cube.faces.find("east"); face != cube.faces.end()) {
						auto uv = face->second.uv;
						auto u = uv.x;
						auto v = uv.y;

						buildFace(builder,
								{x1, y0, z0},
								{x1, y1, z0},
								{x1, y1, z1},
								{x1, y0, z1},
								u / float(texture_width),
								v / float(texture_width),
								(u + size.z) / float(texture_width),
								(v + size.y) / float(texture_width),
								{1.0f, 0, 0}
						);
					}

					if (auto face = cube.faces.find("north"); face != cube.faces.end()) {
						auto uv = face->second.uv;
						auto u = uv.x;
						auto v = uv.y;

						buildFace(builder,
								{x1, y0, z1},
								{x1, y1, z1},
								{x0, y1, z1},
								{x0, y0, z1},
								u / float(texture_width),
								v / float(texture_width),
								(u + size.x) / float(texture_width),
								(v + size.y) / float(texture_width),
								{0, 0, 1.0f}
						);
					}

					if (auto face = cube.faces.find("west"); face != cube.faces.end()) {
						auto uv = face->second.uv;
						auto u = uv.x;
						auto v = uv.y;

						buildFace(builder,
								{x0, y0, z1},
								{x0, y1, z1},
								{x0, y1, z0},
								{x0, y0, z0},
								u / float(texture_width),
								v / float(texture_width),
								(u + size.z) / float(texture_width),
								(v + size.y) / float(texture_width),
								{-1.0f, 0, 0}
						);
					}

					if (auto face = cube.faces.find("up"); face != cube.faces.end()) {
						auto uv = face->second.uv;
						auto u = uv.x;
						auto v = uv.y;

						buildFace(builder,
								{x0, y1, z0},
								{x0, y1, z1},
								{x1, y1, z1},
								{x1, y1, z0},
								u / float(texture_width),
								v / float(texture_width),
								(u + size.x) / float(texture_width),
								(v + size.z) / float(texture_width),
								{0, 1.0f, 0}
						);
					}

					if (auto face = cube.faces.find("down"); face != cube.faces.end()) {
						auto uv = face->second.uv;
						auto u = uv.x;
						auto v = uv.y;

						buildFace(builder,
								{x0, y0, z1},
								{x0, y0, z0},
								{x1, y0, z0},
								{x1, y0, z1},
								u / float(texture_width),
								v / float(texture_width),
								(u + size.x) / float(texture_width),
								(v + size.z) / float(texture_width),
								{0, -1.0f, 0}
						);
					}
				}
			}
		}

		renderBuffer.SetIndexBufferCount(builder.indices.size(), sizeof(int));
		renderBuffer.SetVertexBufferCount(builder.vertices.size(), sizeof(Vertex));

		renderBuffer.SetIndexBufferData(builder.indices.data(), 0, 0, sizeof(int) * builder.indices.size());
		renderBuffer.SetVertexBufferData(builder.vertices.data(), 0, 0, sizeof(Vertex) * builder.vertices.size());
	}

	void buildFace(
		VertexBuilder& builder,
		const Vector3& p1,
		const Vector3& p2,
		const Vector3& p3,
		const Vector3& p4,
		float u0,
		float v0,
		float u1,
		float v1,
		const Vector3& normal
	) {
		builder.addQuad(0, 1, 2, 0, 2, 3);

		builder.vertices.emplace_back(p1.x, p1.y, p1.z, u0, v0, normal.x, normal.y, normal.z);
		builder.vertices.emplace_back(p2.x, p2.y, p2.z, u0, v1, normal.x, normal.y, normal.z);
		builder.vertices.emplace_back(p3.x, p3.y, p3.z, u1, v1, normal.x, normal.y, normal.z);
		builder.vertices.emplace_back(p4.x, p4.y, p4.z, u1, v0, normal.x, normal.y, normal.z);
	}

	void buildQuad(VertexBuilder& builder, const TexturedQuad& quad) {
		auto normal = quad.normal;

		builder.addQuad(0, 1, 2, 0, 2, 3);
		for (auto &&vertex : quad.vertices) {
			builder.vertices.emplace_back(vertex.x, vertex.y, vertex.z, vertex.u, vertex.v, normal.x, normal.y, normal.z);
		}
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
