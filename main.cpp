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
#include "src/client/renderer/EntityRenderer.hpp"
#include "client/renderer/texture/TextureManager.hpp"
#include "client/renderer/texture/TextureAtlas.hpp"
#include "client/renderer/material/MaterialManager.hpp"
#include "client/renderer/model/ModelFormat.hpp"
#include "client/renderer/Colormap.hpp"

#include "client/renderer/BlockTessellator.hpp"

#include "world/tile/Tile.hpp"

#include "nlohmann/json.hpp"

using Json = nlohmann::json;

struct Camera {
	Camera(AppPlatform* platform) {
		auto object = Json::parse(platform->readAssetFile("definitions/cameras/cameras/first_person.json"));

		field_of_view = object.at("field_of_view").get<float>();
		near_clipping_plane = object.at("near_clipping_plane").get<float>();
		far_clipping_plane = object.at("far_clipping_plane").get<float>();
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

struct GameClient {
	RenderSystem* core = RenderSystem::Instance();

	~GameClient() {
		core->device().waitIdle();
	}

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

		atlas = std::make_unique<TextureAtlas>();
		atlas->loadMetaFile(resourceManager);
		textureManager->upload("textures/blocks", atlas);

		loadBlocks();
		Colormap::initColormaps(resourceManager);
		Tile::initTiles(textureManager);

		camera = std::make_unique<Camera>(platform.get());
		gui = std::make_unique<GUI>(window.getPlatformWindow(), renderContext);
	}

	void loadEntities() {
//		auto bytes = resourceManager->loadFile("entity/agent.entity.json").value();
//		auto entity_obj = json::Parser{bytes}.parse().value().as_object().value();
//		auto& client_entity = entity_obj.at("minecraft:client_entity").as_object().value();
//		auto& description = client_entity.at("description").as_object().value();
//		auto& identifier = description.at("identifier").as_string().value();
//		auto& materials = description.at("materials").as_object().value();
//
//		std::unordered_map<std::string, Handle<Material>> render_materials;
//
//		for (auto& [name, material_name] : materials) {
//			render_materials.emplace(std::move(name), materialManager->getMaterial(material_name.as_string().value()));
//		}
	}

	std::map<std::string, std::unique_ptr<ModelFormat>> models;

	void parseModels(const Json& geometries) {
		using namespace std::string_view_literals;

		auto format_version = geometries.at("format_version").get<std::string>();

		if (format_version == "1.8.0"sv || format_version == "1.10.0"sv) {
			for (auto& [name, geometry] : geometries.items()) {
				if (!name.starts_with("geometry.")) continue;

				auto model_format = new ModelFormat();

				auto delim = name.find_first_of(':');
				if (delim != std::string::npos) {
					model_format->name = name.substr(0, delim);
					model_format->parent = name.substr(delim + 1);
				} else {
					model_format->name = name.substr(0);
				}

				model_format->visible_bounds_width = geometry.value("visible_bounds_width", 0);
				model_format->visible_bounds_height = geometry.value("visible_bounds_height", 0);
				model_format->texture_width = geometry.value<int>("texturewidth", 64);
				model_format->texture_height = geometry.value<int>("textureheight", 64);

				auto bones = geometry.find("bones");
				if (bones != geometry.end()) {
					for (auto &bone : *bones) {
						auto bone_format = new ModelBoneFormat();
						bone_format->name = bone.at("name").get<std::string>();
						bone_format->neverRender = bone.value<bool>("neverRender", false);
						bone_format->mirror = bone.value<bool>("mirror", false);
						bone_format->reset = bone.value<bool>("reset", false);
						bone_format->pivot = bone.value("pivot", Vector3{});

						auto cubes = bone.find("cubes");
						if (cubes != bone.end()) {
							bone_format->cubes.reserve(cubes->size());

							for (auto &cube : *cubes) {
								auto &cube_format = bone_format->cubes.emplace_back();

								cube_format.origin = cube.at("origin").get<Vector3>();
								cube_format.size = cube.at("size").get<Vector3>();
								cube_format.uv = cube.value("uv", Vector2{});
							}

							model_format->bones.emplace(bone_format->name, bone_format);
						}
					}
				}

				models.emplace(model_format->name, model_format);
			}
		} else if (format_version == "1.12.0"sv || format_version == "1.16.0"sv) {
			for (auto& geometry : geometries["minecraft:geometry"]) {
				auto& description = geometry.at("description");

				auto model_format = new ModelFormat();

				model_format->name = description.at("identifier").get<std::string>();
				model_format->visible_bounds_width = description.value("visible_bounds_width", 0);
				model_format->visible_bounds_height = description.value("visible_bounds_height", 0);
				model_format->texture_width = description.value<int>("texture_width", 64);
				model_format->texture_height = description.value<int>("texture_height", 64);

				if (auto bones = geometry.find("bones"); bones != geometry.end()) {
					for (auto &bone : *bones) {
						auto bone_format = new ModelBoneFormat();
						bone_format->name = bone.at("name").get<std::string>();
						bone_format->neverRender = bone.value<bool>("neverRender", false);
						bone_format->mirror = bone.value<bool>("mirror", false);
						bone_format->reset = bone.value<bool>("reset", false);
						bone_format->pivot = bone.value("pivot", Vector3{});

						if (auto cubes = bone.find("cubes"); cubes != bone.end()) {
							bone_format->cubes.reserve(cubes->size());

							for (auto &cube : *cubes) {
								auto &cube_format = bone_format->cubes.emplace_back();

								cube_format.origin = cube.at("origin").get<Vector3>();
								cube_format.rotation = cube.value("rotation", Vector3{});
								cube_format.size = cube.at("size").get<Vector3>();

								if (auto uv = cube.find("uv"); uv != cube.end()) {
									if (uv->is_array()) {
										cube_format.uv = uv->get<Vector2>();
									} else {
										cube_format.uv_box = false;

										for (auto& item : uv->items()) {
											auto& face = cube_format.faces[item.key()];
											face.uv = item.value().at("uv").get<Vector2>();
										}
									}
								}
							}

							model_format->bones.emplace(bone_format->name, bone_format);
						}
					}
				}
				models.emplace(model_format->name, model_format);
			}
		} else {
			std::cout << format_version << std::endl;
		}

	}

	void loadModels() {
		std::filesystem::directory_entry directory("models");

		for (auto& resources : resourceManager->getResources("models")) {
			parseModels(Json::parse(resources));
		}

		auto material = materialManager->getMaterial("entity_static");
		material->SetTexture(textureManager->getTexture("textures/entity/bed/white"));

		agentRenderer = std::make_unique<EntityRenderer>(material, models.at("geometry.bed"));
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
		agentRenderer->render(cmd, transform);
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
			float xdelta, ydelta;
			Mouse::getDelta(xdelta, ydelta);

			if (xdelta != 0 || ydelta != 0) {
				double d4 = 0.5f * (double) 0.6F + (double) 0.2F;
				double d5 = d4 * d4 * d4 * 8.0;

				rotationYaw = rotationYaw - xdelta * d5 * dt * 9.0;
				rotationPitch = glm::clamp(rotationPitch - ydelta * d5 * dt * 9.0, -90.0, 90.0);
			}
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