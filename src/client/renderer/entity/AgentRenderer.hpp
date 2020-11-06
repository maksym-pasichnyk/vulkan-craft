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

struct AgentRenderer {
	ModelRenderer agentModel;

	Material* material;

	AgentRenderer(Handle<ResourceManager> resourceManager, Handle<TextureManager> textureManager, Handle<MaterialManager> materialManager) {
		material = materialManager->getMaterial("agent");
		material->SetTexture(textureManager->getTexture("textures/entity/agent"));

		if (auto bytes = resourceManager->loadFile("models/mobs.json")) {
			json::Parser parser(*bytes);

			auto mobs = parser.parse().value();

			auto mob = mobs.get("geometry.agent");
			{
				int texture_width = mob->get("texturewidth")->as_i64().value();
				int texture_height = mob->get("textureheight")->as_i64().value();

				if (auto bones = mob->get("bones")) {
					for (auto& bone : bones->as_array().value()) {
						if (auto cubes = bone.get("cubes")) {
							for (auto& cube : cubes->as_array().value()) {
								auto& origin = cube.get("origin").value().as_array().value();
								auto& size = cube.get("size").value().as_array().value();
								auto& uv = cube.get("uv").value().as_array().value();

//								if (!neverRender) {
									const float posX = origin[0].as_f64().value() / 16.0f;
									const float posY = origin[1].as_f64().value() / 16.0f;
									const float posZ = origin[2].as_f64().value() / 16.0f;

									const float sizeX = size[0].as_f64().value();
									const float sizeY = size[1].as_f64().value();
									const float sizeZ = size[2].as_f64().value();

									const float x0 = posX;
									const float y0 = posY;
									const float z0 = posZ;

									const float x1 = posX + sizeX / 16.0f;
									const float y1 = posY + sizeY / 16.0f;
									const float z1 = posZ + sizeZ / 16.0f;

									const float u = uv[0].as_f64().value();
									const float v = uv[1].as_f64().value();

									const float u0 = (u) / texture_width;
									const float u1 = (u + sizeZ) / texture_width;
									const float u2 = (u + sizeZ + sizeX) / texture_width;
									const float u3 = (u + sizeZ + sizeX + sizeX) / texture_width;
									const float u4 = (u + sizeZ + sizeX + sizeZ) / texture_width;
									const float u5 = (u + sizeZ + sizeX + sizeZ + sizeX) / texture_width;

									const float v0 = (v) / texture_height;
									const float v1 = (v + sizeZ) / texture_height;
									const float v2 = (v + sizeZ + sizeY) / texture_height;

									TexturedQuad quad0 {
									    .vertices{
									        PositionTextureVertex{x0, y0, z0, u1, v2},
									        PositionTextureVertex{x0, y1, z0, u1, v1},
									        PositionTextureVertex{x1, y1, z0, u2, v1},
									        PositionTextureVertex{x1, y0, z0, u2, v2}
                                        },
									    .normal{0, 0, -1.0f}
                                    };
									TexturedQuad quad1 {
                                        .vertices{
                                            PositionTextureVertex{x1, y0, z0, u1, v2},
                                            PositionTextureVertex{x1, y1, z0, u1, v1},
                                            PositionTextureVertex{x1, y1, z1, u0, v1},
                                            PositionTextureVertex{x1, y0, z1, u0, v2}
                                        },
                                        .normal{1.0f, 0, 0}
									};
									TexturedQuad quad2 {
                                        .vertices{
                                            PositionTextureVertex{x1, y0, z1, u4, v2},
                                            PositionTextureVertex{x1, y1, z1, u4, v1},
                                            PositionTextureVertex{x0, y1, z1, u5, v1},
                                            PositionTextureVertex{x0, y0, z1, u5, v2}
                                        },
                                        .normal{0, 0, 1.0f}
									};
									TexturedQuad quad3 {
                                        .vertices{
                                            PositionTextureVertex{x0, y0, z1, u0, v2},
                                            PositionTextureVertex{x0, y1, z1, u0, v1},
                                            PositionTextureVertex{x0, y1, z0, u1, v1},
                                            PositionTextureVertex{x0, y0, z0, u1, v2}
                                        },
                                        .normal{-1.0f, 0, 0}
									};
									TexturedQuad quad4 {
                                        .vertices{
                                            PositionTextureVertex{x0, y1, z0, u1, v1},
                                            PositionTextureVertex{x0, y1, z1, u1, v0},
                                            PositionTextureVertex{x1, y1, z1, u2, v0},
                                            PositionTextureVertex{x1, y1, z0, u2, v1}
                                        },
                                        .normal{0, 1.0f, 0}
									};
									TexturedQuad quad5 {
                                        .vertices{
                                            PositionTextureVertex{x0, y0, z1, u2, v1},
                                            PositionTextureVertex{x0, y0, z0, u2, v0},
                                            PositionTextureVertex{x1, y0, z0, u3, v0},
                                            PositionTextureVertex{x1, y0, z1, u3, v1}
                                        },
                                        .normal{0, -1.0f, 0}
									};

//									TexturedQuad quad0({vertex0, vertex1, vertex2, vertex3}, u1, v1, u2, v2, texture_width, texture_height, {0, 0, -1});
//									TexturedQuad quad1({vertex3, vertex2, vertex4, vertex5}, u1, v1, u0, v2, texture_width, texture_height, {1, 0, 0});
//									TexturedQuad quad2({vertex5, vertex4, vertex6, vertex7}, u4, v1, u5, v2, texture_width, texture_height, {0, 0, 1});
//									TexturedQuad quad3({vertex7, vertex6, vertex1, vertex0}, u0, v1, u1, v2, texture_width, texture_height, {-1, 0, 0});
//									TexturedQuad quad4({vertex1, vertex6, vertex4, vertex2}, u1, v0, u2, v1, texture_width, texture_height, {0, 1, 0});
//									TexturedQuad quad5({vertex7, vertex0, vertex3, vertex5}, u2, v0, u3, v1, texture_width, texture_height, {0, -1, 0});

									ModelBox modelBox{
										.quads {
											quad0,
											quad1,
											quad2,
											quad3,
											quad4,
											quad5
										}
									};

									agentModel.cubes.push_back(modelBox);
//								}
							}
						}
					}
				}
			}
		};

		for (auto&& cube : agentModel.cubes) {
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

	~AgentRenderer() {
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
