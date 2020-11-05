#pragma once

#include <unordered_set>
#include <cstring>

#include "renderer/RenderSystem.hpp"

enum class CursorState {
	Normal,
	Locked
};

//struct Input {
////	void setCursorState(CursorState state) {
////		if (state == _cursorState)
////			return;
////
////		_cursorState = state;
////		switch (state) {
////		case CursorState::Normal:
////			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
////			break;
////		case CursorState::Locked:
////			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
////			break;
////		}
////		double xpos, ypos;
////		glfwGetCursorPos(_window, &xpos, &ypos);
////		_mousePosition[0] = static_cast<float>(xpos);
////		_mousePosition[1] = static_cast<float>(ypos);
////		_delta[0] = 0;
////		_delta[1] = 0;
////	}
//
//private:
//	GLFWwindow* _window;
//
//	CursorState _cursorState;
//};
