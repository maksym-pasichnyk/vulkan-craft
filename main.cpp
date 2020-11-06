#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "client/AppPlatform.hpp"

#include "client/GameWindow.hpp"
#include "client/Clock.hpp"
#include "client/Mouse.hpp"
#include "client/Keyboard.hpp"
#include "client/gui/gui.hpp"

#include "resources/ResourceManager.hpp"

#include "client/renderer/RenderContext.hpp"
#include "client/renderer/entity/EntityRenderer.hpp"
#include "client/renderer/texture/TextureManager.hpp"
#include "client/renderer/texture/TextureAtlas.hpp"
#include "client/renderer/material/MaterialManager.hpp"
#include "client/renderer/model/ParsedModelGeometry.hpp"

#include "client/renderer/BlockTessellator.hpp"

#include "world/tile/Tile.hpp"


struct Camera {
	Camera(AppPlatform* platform) {
		auto bytes = platform->readAssetFile("definitions/cameras/cameras/first_person.json");

		json::Parser parser(bytes);
		auto object = parser.parse().value().as_object().value();

		field_of_view = object.at("field_of_view").as_f64().value();
		near_clipping_plane = object.at("near_clipping_plane").as_f64().value();
		far_clipping_plane = object.at("far_clipping_plane").as_f64().value();
	}

	void setRenderSize(int width, int height) {
		const float tanHalfFovy = glm::tan(glm::radians(field_of_view) * 0.5f);
		const float aspect_ratio = float(width) / float(height);

		const float a = 1.0f / (aspect_ratio * tanHalfFovy);
		const float b = -1.0f / tanHalfFovy;
		const float c = far_clipping_plane / (far_clipping_plane - near_clipping_plane);
		const float d = (near_clipping_plane * far_clipping_plane) / (near_clipping_plane - far_clipping_plane);

		_projection = {
				a, 0, 0, 0,
				0, b, 0, 0,
				0, 0, c, 1,
				0, 0, d, 0
		};
	}

	glm::mat4 projection() {
		return _projection;
	}

private:
	float field_of_view;
	float near_clipping_plane;
	float far_clipping_plane;

	glm::mat4 _projection;
};

struct TileTextures {
	std::optional<std::string> up;
	std::optional<std::string> down;
	std::optional<std::string> north;
	std::optional<std::string> south;
	std::optional<std::string> east;
	std::optional<std::string> west;

	void read(json::Value& textures) {
		if (auto textureObj = textures.as_string()) {
			auto& texture = textureObj.value();
			up = texture;
			down = texture;
			north = texture;
			south = texture;
			east = texture;
			west = texture;
		} else {
			auto side = textures.get("side");

			up = get(textures, "up");
			down = get(textures, "down");
			north = get(textures, "north", side);
			south = get(textures, "north", side);
			east = get(textures, "east", side);
			west = get(textures, "west", side);
		}
	}

private:
	static std::string get(json::Value& textures, const std::string& name) {
		return textures.get(name).value().as_string().value();
	}

	static std::string get(json::Value& textures, const std::string& name, json::Result<json::Value> other) {
		if (auto texture = textures.get(name)) {
			return texture.value().as_string().value();
		}
		return other.value().as_string().value();
	}
};

struct GameClient {
	RenderSystem* core = RenderSystem::Instance();

	void init(GameWindow& window) {
		core->init(window.getPlatformWindow());

		platform = std::make_unique<AppPlatform>();
		renderContext = std::make_unique<RenderContext>();

		resourceManager = std::make_unique<ResourceManager>();
		textureManager = std::make_unique<TextureManager>(renderContext, resourceManager);
		materialManager = std::make_unique<MaterialManager>();

		resourceManager->addResourcePack(std::make_unique<ResourcePack>("assets/resource_packs/vanilla"));
		materialManager->loadMetaFile(platform, renderContext);

		loadEntities();
		loadModels();

		auto material = materialManager->getMaterial("agent");
		material->SetTexture(textureManager->getTexture("textures/entity/agent"));



//		agentRenderer = std::make_unique<EntityRenderer>(resourceManager, material);

		atlas = std::make_unique<TextureAtlas>();
		atlas->loadMetaFile(resourceManager);
		textureManager->upload("textures/blocks", atlas);

		loadBlocks();
		Tile::initTiles(textureManager);

		camera = std::make_unique<Camera>(platform.get());
		gui = std::make_unique<GUI>(window.getPlatformWindow(), renderContext);
	}

	void loadEntities() {
		auto bytes = resourceManager->loadFile("entity/agent.entity.json").value();
		auto entity_obj = json::Parser{bytes}.parse().value().as_object().value();
		auto& client_entity = entity_obj.at("minecraft:client_entity").as_object().value();
		auto& description = client_entity.at("description").as_object().value();
		auto& identifier = description.at("identifier").as_string().value();
		auto& materials = description.at("materials").as_object().value();

		std::unordered_map<std::string, Handle<Material>> render_materials;

		for (auto& [name, material_name] : materials) {
			render_materials.emplace(std::move(name), materialManager->getMaterial(material_name.as_string().value()));
		}
	}

	void loadModels() {
		auto mobs = json::Parser{resourceManager->loadFile("models/mobs.json").value()}.parse().value().as_object().value();
		auto& geometry = mobs.at("geometry.agent");

		int texture_width = geometry.get("texturewidth")->as_i64().value();
		int texture_height = geometry.get("textureheight")->as_i64().value();

		ParsedModelGeometry parsedGeometry;

		if (auto bones = geometry.get("bones")) {
			for (auto& bone : bones->as_array().value()) {
				ParsedGeometryBoneNode node;

				if (auto cubes = bone.get("cubes")) {
					ParsedGeometryBoneElement element;

					for (auto& cube : cubes->as_array().value()) {
						auto& origin = cube.get("origin").value().as_array().value();
						auto& size = cube.get("size").value().as_array().value();
						auto& uv = cube.get("uv").value().as_array().value();

						element.origin.x = origin[0].as_f64().value();
						element.origin.y = origin[1].as_f64().value();
						element.origin.z = origin[2].as_f64().value();

						element.size.x = size[0].as_f64().value();
						element.size.y = size[1].as_f64().value();
						element.size.z = size[2].as_f64().value();

						element.uv = Vector2 {
							(float) uv[0].as_f64().value(),
							(float) uv[1].as_f64().value()
						};

						node.cubes.emplace_back(std::move(element));

//								if (!neverRender) {
//							const float posX = origin[0].as_f64().value() / 16.0f;
//							const float posY = origin[1].as_f64().value() / 16.0f;
//							const float posZ = origin[2].as_f64().value() / 16.0f;
//
//							const float sizeX = size[0].as_f64().value();
//							const float sizeY = size[1].as_f64().value();
//							const float sizeZ = size[2].as_f64().value();
//
//							const float x0 = posX;
//							const float y0 = posY;
//							const float z0 = posZ;
//
//							const float x1 = posX + sizeX / 16.0f;
//							const float y1 = posY + sizeY / 16.0f;
//							const float z1 = posZ + sizeZ / 16.0f;
//
//							const float u = uv[0].as_f64().value();
//							const float v = uv[1].as_f64().value();
//
//							const float u0 = (u) / texture_width;
//							const float u1 = (u + sizeZ) / texture_width;
//							const float u2 = (u + sizeZ + sizeX) / texture_width;
//							const float u3 = (u + sizeZ + sizeX + sizeX) / texture_width;
//							const float u4 = (u + sizeZ + sizeX + sizeZ) / texture_width;
//							const float u5 = (u + sizeZ + sizeX + sizeZ + sizeX) / texture_width;
//
//							const float v0 = (v) / texture_height;
//							const float v1 = (v + sizeZ) / texture_height;
//							const float v2 = (v + sizeZ + sizeY) / texture_height;
//
//							TexturedQuad quad0 {
//								.vertices{
//									PositionTextureVertex{x0, y0, z0, u1, v2},
//									PositionTextureVertex{x0, y1, z0, u1, v1},
//									PositionTextureVertex{x1, y1, z0, u2, v1},
//									PositionTextureVertex{x1, y0, z0, u2, v2}
//								},
//								.normal{0, 0, -1.0f}
//							};
//							TexturedQuad quad1 {
//								.vertices{
//									PositionTextureVertex{x1, y0, z0, u1, v2},
//									PositionTextureVertex{x1, y1, z0, u1, v1},
//									PositionTextureVertex{x1, y1, z1, u0, v1},
//									PositionTextureVertex{x1, y0, z1, u0, v2}
//								},
//								.normal{1.0f, 0, 0}
//							};
//							TexturedQuad quad2 {
//								.vertices{
//									PositionTextureVertex{x1, y0, z1, u4, v2},
//									PositionTextureVertex{x1, y1, z1, u4, v1},
//									PositionTextureVertex{x0, y1, z1, u5, v1},
//									PositionTextureVertex{x0, y0, z1, u5, v2}
//								},
//								.normal{0, 0, 1.0f}
//							};
//							TexturedQuad quad3 {
//								.vertices{
//									PositionTextureVertex{x0, y0, z1, u0, v2},
//									PositionTextureVertex{x0, y1, z1, u0, v1},
//									PositionTextureVertex{x0, y1, z0, u1, v1},
//									PositionTextureVertex{x0, y0, z0, u1, v2}
//								},
//								.normal{-1.0f, 0, 0}
//							};
//							TexturedQuad quad4 {
//								.vertices{
//									PositionTextureVertex{x0, y1, z0, u1, v1},
//									PositionTextureVertex{x0, y1, z1, u1, v0},
//									PositionTextureVertex{x1, y1, z1, u2, v0},
//									PositionTextureVertex{x1, y1, z0, u2, v1}
//								},
//								.normal{0, 1.0f, 0}
//							};
//							TexturedQuad quad5 {
//								.vertices{
//									PositionTextureVertex{x0, y0, z1, u2, v1},
//									PositionTextureVertex{x0, y0, z0, u2, v0},
//									PositionTextureVertex{x1, y0, z0, u3, v0},
//									PositionTextureVertex{x1, y0, z1, u3, v1}
//								},
//								.normal{0, -1.0f, 0}
//							};
//
//							ModelBox modelBox{
//								.quads {
//									quad0,
//									quad1,
//									quad2,
//									quad3,
//									quad4,
//									quad5
//								}
//							};
//
//							renderer->cubes.push_back(modelBox);
//								}
					}
				}

				parsedGeometry.bones.emplace_back(std::move(node));
			}
		}
	}

	void loadBlocks() {
//		auto bytes = resourceManager->loadFile("blocks.json");
//		auto object = json::Parser{bytes.value()}.parse().value().as_object().value();
//		auto format_version = object.extract("format_version");
//
//		for (auto& [name, obj] : object) {
//			TileTextures tileTextures;
//
//			if (auto textures = obj.get("textures")) {
////				std::cout << name << std::endl;
//
//				tileTextures.read(textures.value());
//			}
//		}
	}

	void tick() {
		clock.update();

		Mouse::update();
		Keyboard::update();

		glfwPollEvents();

		camera_tick(clock.deltaSeconds());
	}

	void render() {
		auto proj = camera->projection();
		auto view = rotationMatrix();

		CameraTransform transform {
			.camera = proj * glm::translate(view, -position)
		};

		gui->begin();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("Test", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
//		ImGui::Image(atlas->renderTexture, ImVec2(512, 512));
		ImGui::End();
		gui->end();

		auto cmd = renderContext->begin();
		cmd.setViewport(0, 1, &viewport);
		cmd.setScissor(0, 1, &scissor);
//		agentRenderer->render(cmd, transform);
		gui->draw(cmd);
		renderContext->end();
	}

	void setRenderSize(int width, int height) {
		renderContext->setRenderSize(width, height);
		camera->setRenderSize(width, height);

		viewport.width = width;
		viewport.height = height;

		scissor.extent.width = width;
		scissor.extent.height = height;
	}

	void quit() {
		_running = false;
	}

	bool wantToQuit() {
		return !_running;
	}
private:
	glm::mat4 rotationMatrix() {
		float yaw = glm::radians(rotationYaw);
		float pitch = glm::radians(rotationPitch);

		float sp = glm::sin(pitch);
		float cp = glm::cos(pitch);

		float c = glm::cos(yaw);
		float s = glm::sin(yaw);

		return {
			c, sp * s, -cp * s, 0,
			0, cp, sp, 0,
			s, -sp * c, cp * c, 0,
			0, 0, 0, 1
		};
	}

	void camera_tick(float dt) {
		if (Mouse::IsButtonPressed(MouseButton::Left)) {
//			_input->setCursorState(CursorState::Locked);

			float xdelta, ydelta;
			Mouse::getDelta(xdelta, ydelta);

			if (xdelta != 0 || ydelta != 0) {
				double d4 = 0.5f * (double) 0.6F + (double) 0.2F;
				double d5 = d4 * d4 * d4 * 8.0;

				rotationYaw = rotationYaw - xdelta * d5 * dt * 9.0;
				rotationPitch = glm::clamp(rotationPitch - ydelta * d5 * dt * 9.0, -90.0, 90.0);
			}
		} else {
//			_input->setCursorState(CursorState::Normal);
		}

		auto rot = glm::mat3(rotationMatrix());
		auto forward = glm::vec3(0, 0, 1) * rot;
		auto right = glm::vec3(1, 0, 0) * rot;

		if (Keyboard::IsKeyPressed(Key::W)) {
			position += forward * dt * 5.0f;
		}
		if (Keyboard::IsKeyPressed(Key::S)) {
			position -= forward * dt * 5.0f;
		}
		if (Keyboard::IsKeyPressed(Key::A)) {
			position -= right * dt * 5.0f;
		}
		if (Keyboard::IsKeyPressed(Key::D)) {
			position += right * dt * 5.0f;
		}
	}
private:
	Clock clock;

	std::unique_ptr<AppPlatform> platform;
	std::unique_ptr<RenderContext> renderContext;
	std::unique_ptr<ResourceManager> resourceManager;
	std::unique_ptr<MaterialManager> materialManager;
	std::unique_ptr<TextureManager> textureManager;
	std::unique_ptr<EntityRenderer> agentRenderer;
	std::unique_ptr<TextureAtlas> atlas;

	std::unique_ptr<Camera> camera;
	std::unique_ptr<GUI> gui;

	vk::Viewport viewport{
		.x = 0,
		.y = 0,
		.width = 0,
		.height = 0,
		.minDepth = 0,
		.maxDepth = 1
	};

	vk::Rect2D scissor {};

	glm::vec3 position{0, 1.5f, -2};
	float rotationYaw{0};
	float rotationPitch{0};

	bool _running{true};
};

int main(int, char**) {
	GameWindow window("Vulkan", 1280, 720);
	window.setMouseButtonCallback(&Mouse::handleMouseButton);
	window.setMousePositionCallback(&Mouse::handleMousePosition);
	window.setKeyboardCallback(&Keyboard::handleKeyInput);

	int width, height;
	window.getWindowSize(width, height);

	GameClient client{};
	client.init(window);
	client.setRenderSize(width, height);

	while (!window.shouldClose()) {
		if (client.wantToQuit()) {
			window.close();
			break;
		}

		client.tick();
		client.render();
	}
	return 0;
}