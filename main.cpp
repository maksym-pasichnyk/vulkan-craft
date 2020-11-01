#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <client/AppPlatform.hpp>

#include "GameWindow.hpp"
#include "renderer/RenderContext.hpp"

#include "resources/ResourceManager.hpp"
#include "renderer/texture/TextureManager.hpp"

#include "Clock.hpp"
#include "Mouse.hpp"
#include "Keyboard.hpp"

#include "renderer/entity/AgentRenderer.hpp"

#include "world/tile/Tile.hpp"

enum class ConnectionBit {
    None = 0,
    West = 1,
    North = 2,
    East = 4,
    South = 8,
};

inline constexpr auto operator&&(ConnectionBit __lhs, ConnectionBit __rhs) -> bool {
    using base = std::underlying_type<ConnectionBit>::type;
    return (base(__lhs) & base(__rhs)) != base(0);
}

inline constexpr auto operator|(ConnectionBit __lhs, ConnectionBit __rhs) -> ConnectionBit {
    using base = std::underlying_type<ConnectionBit>::type;
    return ConnectionBit(base(__lhs) | base(__rhs));
}

enum class FlammableBit {
    Up = 1,
    Down = 2,
    North = 4,
    South = 8,
    East = 16,
    West = 32
};

inline constexpr auto operator|(FlammableBit __lhs, FlammableBit __rhs) -> FlammableBit {
    using base = std::underlying_type<FlammableBit>::type;
    return FlammableBit(base(__lhs) | base(__rhs));
}

inline constexpr auto operator|=(FlammableBit& __lhs, FlammableBit __rhs) -> FlammableBit& {
    using base = std::underlying_type<FlammableBit>::type;
    return __lhs = FlammableBit(base(__lhs) | base(__rhs));
}

struct Camera {
	Camera(AppPlatform* platform) {
		auto bytes = platform->readAssetFile("definitions/cameras/cameras/first_person.json");

		json::Parser parser(bytes);
		auto object = parser.parse().value().as_object().value();

		field_of_view = object.at("field_of_view").as_f64().value();
		near_clipping_plane = object.at("near_clipping_plane").as_f64().value();
		far_clipping_plane = object.at("far_clipping_plane").as_f64().value();
	}

	void setRenderingSize(int width, int height) {
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

//struct TextureSlot {
//	consteval TextureSlot(std::string_view name) : name(name), parent(nullptr) {}
//	consteval TextureSlot(std::string_view name, TextureSlot& parent) : name(name), parent(&parent) {}
//
//private:
//	std::string_view name;
//	TextureSlot* parent;
//};
//
//struct TextureSlots {
//	inline static constinit TextureSlot Texture = TextureSlot("texture");
//	inline static constinit TextureSlot Side = TextureSlot("side", Texture);
//	inline static constinit TextureSlot Top = TextureSlot("top", Texture);
//	inline static constinit TextureSlot Bottom = TextureSlot("bottom", Texture);
//	inline static constinit TextureSlot North = TextureSlot("north", Side);
//	inline static constinit TextureSlot South = TextureSlot("south", Side);
//	inline static constinit TextureSlot East = TextureSlot("east", Side);
//	inline static constinit TextureSlot West = TextureSlot("west", Side);
//};

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
		textureManager = std::make_unique<TextureManager>(renderContext.get(), resourceManager.get());
		resourceManager->addResourcePack(std::make_unique<ResourcePack>("assets/resource_packs/vanilla"));

		Tile::initTiles(textureManager.get());

		agent = std::make_unique<AgentRenderer>(platform.get(), resourceManager.get(), textureManager.get(), renderContext.get());

		camera = std::make_unique<Camera>(platform.get());

		auto bytes = resourceManager->loadFile("blocks.json");
		json::Parser parser{bytes.value()};

		auto object = parser.parse().value().as_object().value();
		auto format_version = object.extract("format_version");

		for (auto& [name, obj] : object) {
			TileTextures tileTextures;

			if (auto textures = obj.get("textures")) {
				std::cout << name << std::endl;

				tileTextures.read(textures.value());
			}
		}
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

		auto cmd = renderContext->begin();
		cmd.setViewport(0, 1, &viewport);
		cmd.setScissor(0, 1, &scissor);

		agent->render(cmd, transform);
		renderContext->end();
	}

	void setRenderingSize(int width, int height) {
		renderContext->setRenderingSize(width, height);
		camera->setRenderingSize(width, height);

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

	std::unique_ptr<ResourceManager> resourceManager;
	std::unique_ptr<TextureManager> textureManager;
	std::unique_ptr<RenderContext> renderContext;
	std::unique_ptr<AgentRenderer> agent;

	std::unique_ptr<Camera> camera;

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
	client.setRenderingSize(width, height);

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