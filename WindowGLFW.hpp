#pragma once

#include <GLFW/glfw3.h>

struct WindowGLFW {
	void create(int width, int height, const char* title) {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		_window = glfwCreateWindow(width, height, title, NULL, NULL);

		glfwGetFramebufferSize(_window, &_width, &_height);

	}
	void destroy() {
		glfwDestroyWindow(_window);
		glfwTerminate();
	}

	int width() const { return _width; }
	int height() const { return _height; }

//private:
	GLFWwindow* _window{nullptr};
	int _width{0}, _height{0};
};