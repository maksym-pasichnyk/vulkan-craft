#pragma once

#include <GLFW/glfw3.h>

enum class KeyAction {
	Release = 0,
	Press = 1,
	Repeat = 2,
};
enum class MouseButtonAction {
    Release = 0,
	Press = 1,
};

struct GameWindow {
private:
	using WindowSizeCallback = std::function<void(int, int)>;
	using MouseButtonCallback = std::function<void(double, double, int, MouseButtonAction)>;
	using MousePositionCallback = std::function<void(double, double)>;
	using MouseScrollCallback = std::function<void(double, double, double, double)>;
	using KeyboardCallback = std::function<void(int, KeyAction)>;
	using KeyboardTextCallback = std::function<void(unsigned int)>;
	using PasteCallback = std::function<void(std::string const &)>;
	using CloseCallback = std::function<void()>;

public:
	GameWindow(const char* title, int width, int height) {
		glfwInit();
		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, title, NULL, NULL);

		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, _glfwWindowSizeCallback);
		glfwSetCursorPosCallback(window, _glfwCursorPosCallback);
		glfwSetMouseButtonCallback(window, _glfwMouseButtonCallback);
		glfwSetWindowCloseCallback(window, _glfwWindowCloseCallback);
		glfwSetKeyCallback(window, _glfwKeyCallback);
		glfwSetCharCallback(window, _glfwCharCallback);
	}
	~GameWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	bool wantToQuit() {
		return glfwWindowShouldClose(window);
	}

	void getWindowSize(int& width, int& height) const {
		glfwGetFramebufferSize(window, &width, &height);
	}

	void close() {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	void setSetInputMode(bool disabled) {
		glfwSetInputMode(window, GLFW_CURSOR, disabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
	}

	GLFWwindow* getPlatformWindow() {
		return window;
	}

	template <typename Callback>
	void setWindowSizeCallback(Callback&& callback) {
		windowSizeCallback = std::forward<Callback>(callback);
	}

	template <typename Callback>
	void setMouseButtonCallback(Callback&& callback) {
		mouseButtonCallback = std::forward<Callback>(callback);
	}

	template <typename Callback>
	void setMousePositionCallback(Callback&& callback) {
		mousePositionCallback = std::forward<Callback>(callback);
	}

	template <typename Callback>
	void setMouseScrollCallback(Callback&& callback) {
		mouseScrollCallback = std::forward<Callback>(callback);
	}

	template <typename Callback>
	void setKeyboardCallback(Callback&& callback) {
		keyboardCallback = std::forward<Callback>(callback);
	}

	template <typename Callback>
	void setKeyboardTextCallback(Callback&& callback) {
		keyboardTextCallback = std::forward<Callback>(callback);
	}

	template <typename Callback>
	void setPasteCallback(Callback&& callback) {
		pasteCallback = std::forward<Callback>(callback);
	}

	template <typename Callback>
	void setMouseRelativePositionCallback(Callback&& callback) {
		mouseRelativePositionCallback = std::forward<Callback>(callback);
	}

	template <typename Callback>
	void setCloseCallback(Callback&& callback) {
		closeCallback = std::forward<Callback>(callback);
	}

private:
	void onWindowSizeChanged(int w, int h) {
		if (windowSizeCallback != nullptr) {
			windowSizeCallback(w, h);
		}
	}

	void onMouseButton(double x, double y, int button, MouseButtonAction action) {
		if (mouseButtonCallback != nullptr) {
			mouseButtonCallback(x, y, button, action);
		}
	}

	void onMousePosition(double x, double y) {
		if (mousePositionCallback != nullptr) {
			mousePositionCallback(x, y);
		}
	}

	void onMouseRelativePosition(double x, double y) {
		if (mouseRelativePositionCallback != nullptr) {
			mouseRelativePositionCallback(x, y);
		}
	}

	void onMouseScroll(double x, double y, double dx, double dy) {
		if (mouseScrollCallback != nullptr) {
			mouseScrollCallback(x, y, dx, dy);
		}
	}

	void onKeyboard(int key, KeyAction action) {
		if (keyboardCallback != nullptr) {
			keyboardCallback(key, action);
		}
	}

	void onKeyboardText(unsigned int c) {
		if (keyboardTextCallback != nullptr) {
			keyboardTextCallback(c);
		}
	}

	void onPaste(std::string const &c) {
		if (pasteCallback != nullptr) {
			pasteCallback(c);
		}
	}

	void onClose() {
		if (closeCallback != nullptr) {
			closeCallback();
		}
	}

	static void _glfwWindowSizeCallback(GLFWwindow* window, int w, int h) {
		auto self = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));
		self->onWindowSizeChanged(w, h);
	}
    static void _glfwCursorPosCallback(GLFWwindow* window, double x, double y) {
		auto self = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));

		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			self->onMouseRelativePosition(x - self->lastMouseX, y - self->lastMouseY);
			self->lastMouseX = x;
			self->lastMouseY = y;
		} else {
			self->onMousePosition(x, y);
		}
    }
    static void _glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		double x, y;
    	glfwGetCursorPos(window, &x, &y);

		auto self = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));
    	self->onMouseButton(x, y, button + 1, static_cast<MouseButtonAction>(action));
    }

    static void _glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto self = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));
		self->onKeyboard(key, static_cast<KeyAction>(action));
    }
    static void _glfwCharCallback(GLFWwindow* window, unsigned int ch) {
		auto self = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));
		self->onKeyboardText(ch);
    }

    static void _glfwWindowCloseCallback(GLFWwindow* window) {
		auto self = static_cast<GameWindow*>(glfwGetWindowUserPointer(window));
    }

private:
	GLFWwindow* window;

    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

	WindowSizeCallback windowSizeCallback;
	MouseButtonCallback mouseButtonCallback;
	MousePositionCallback mousePositionCallback;
	MousePositionCallback mouseRelativePositionCallback;
	MouseScrollCallback mouseScrollCallback;
	KeyboardCallback keyboardCallback;
	KeyboardTextCallback keyboardTextCallback;
	PasteCallback pasteCallback;
	CloseCallback closeCallback;
};