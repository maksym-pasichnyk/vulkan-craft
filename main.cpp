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

inline static glm::mat4 PerspectiveProjectionMatrix(float field_of_view, float aspect_ratio, float near_plane, float far_plane) {
	const float tanHalfFovy = glm::tan(field_of_view * 0.5f);

	const float a = 1.0f / (aspect_ratio * tanHalfFovy);
	const float b = -1.0f / tanHalfFovy;
	const float c = far_plane / (far_plane - near_plane);
	const float d = (near_plane * far_plane) / (near_plane - far_plane);



	return {
		a, 0, 0, 0,
		0, b, 0, 0,
		0, 0, c, 1,
		0, 0, d, 0
	};
}

struct TilePos {
	int x, y, z;
};

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

inline constexpr auto operator&&(FlammableBit __lhs, FlammableBit __rhs) -> bool {
    using base = std::underlying_type<FlammableBit>::type;
    return (base(__lhs) & base(__rhs)) != base(0);
}

inline constexpr auto operator|(FlammableBit __lhs, FlammableBit __rhs) -> FlammableBit {
    using base = std::underlying_type<FlammableBit>::type;
    return FlammableBit(base(__lhs) | base(__rhs));
}

inline constexpr auto operator|=(FlammableBit& __lhs, FlammableBit __rhs) -> FlammableBit& {
    using base = std::underlying_type<FlammableBit>::type;
    return __lhs = FlammableBit(base(__lhs) | base(__rhs));
}

struct GameClient {
	RenderSystem* core = RenderSystem::Instance();

	void init(GameWindow& window) {
		core->init(window.getPlatformWindow());

		platform = std::make_unique<AppPlatform>();
		renderContext = std::make_unique<RenderContext>();

		resourceManager = std::make_unique<ResourceManager>();
		textureManager = std::make_unique<TextureManager>(renderContext.get(), resourceManager.get());

		resourceManager->addResourcePack(std::make_unique<ResourcePack>("assets/resource_packs/vanilla"));

		agent = std::make_unique<AgentRenderer>(platform.get(), resourceManager.get(), textureManager.get(), renderContext.get());
	}

	void tick() {
		clock.update();

		Mouse::update();
		Keyboard::update();

		glfwPollEvents();

		camera_tick(clock.deltaSeconds());
	}

	void render() {
		auto proj = projection;
		auto view = rotationMatrix();

		CameraTransform transform {
			.camera = proj * glm::translate(view, -cameraPosition)
		};

		auto cmd = renderContext->begin();
		agent->render(cmd, transform);
		renderContext->end();
	}

	void setRenderingSize(int width, int height) {
		projection = PerspectiveProjectionMatrix(
				glm::radians(60.0f),
				float(width) / float(height),
				0.1f,
				1000.0f
		);

		agent->setRenderingSize(width, height);
		renderContext->setRenderingSize(width, height);
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

		glm::mat4 view;
		view[0] = glm::vec4(c, sp * s, -cp * s, 0);
		view[1] = glm::vec4(0, cp, sp, 0);
		view[2] = glm::vec4(s, -sp * c, cp * c, 0);
		view[3] = glm::vec4(0, 0, 0, 1);
		return view;
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
			cameraPosition += forward * dt * 5.0f;
		}
		if (Keyboard::IsKeyPressed(Key::S)) {
			cameraPosition -= forward * dt * 5.0f;
		}
		if (Keyboard::IsKeyPressed(Key::A)) {
			cameraPosition -= right * dt * 5.0f;
		}
		if (Keyboard::IsKeyPressed(Key::D)) {
			cameraPosition += right * dt * 5.0f;
		}
	}
private:
	Clock clock;

	std::unique_ptr<AppPlatform> platform;

	std::unique_ptr<ResourceManager> resourceManager;
	std::unique_ptr<TextureManager> textureManager;
	std::unique_ptr<RenderContext> renderContext;
	std::unique_ptr<AgentRenderer> agent;

	glm::mat4 projection;
	glm::vec3 cameraPosition{0, 1.5f, -2};
	float rotationYaw{0};
	float rotationPitch{0};

	bool _running{true};
};

int main(int, char**) {
//	ResourcePack resourcePack("assets/resource_packs/vanilla");
//	resourcePack.LoadResources("models/entity", LoadGeometry);
//	resourcePack.LoadResource("models/mobs.json", LoadGeometry);

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