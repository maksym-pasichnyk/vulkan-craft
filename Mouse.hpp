#pragma once

#include <GameWindow.hpp>

enum class MouseButton : int {
	Button1 = GLFW_MOUSE_BUTTON_1,
	Button2 = GLFW_MOUSE_BUTTON_2,
	Button3 = GLFW_MOUSE_BUTTON_3,
	Button4 = GLFW_MOUSE_BUTTON_4,
	Button5 = GLFW_MOUSE_BUTTON_5,
	Button6 = GLFW_MOUSE_BUTTON_6,
	Button7 = GLFW_MOUSE_BUTTON_7,
	Button8 = GLFW_MOUSE_BUTTON_8,
	Right = Button1,
	Left = Button2,
	Middle = Button3
};

struct Mouse {
public:
	inline static bool IsButtonDown(MouseButton button) {
		return _mouseDown[int(button)] && !_mouseDownPrev[int(button)];
	}

	inline bool static IsButtonPressed(MouseButton button) {
		return _mouseDown[int(button)];
	}

	inline bool static IsButtonReleased(MouseButton button) {
		return _mouseDownPrev[int(button)] && !_mouseDown[int(button)];
	}

public:
	inline static void update() {
		_delta[0] = 0;
		_delta[1] = 0;

		for (int i = 0; i < 5; i++) {
			_mouseDownPrev[i] = _mouseDown[i];
		}
	}

	inline static void handleMouseButton(double xpos, double ypos, int button, MouseButtonAction action) {
		if (action == MouseButtonAction::Press) {
			_mouseDown[button] = true;
		}
		if (action == MouseButtonAction::Release) {
			_mouseDown[button] = false;
		}
	}

	inline static void handleMousePosition(double xpos, double ypos) {
		_delta[0] = static_cast<float>(xpos) - _mousePosition[0];
		_delta[1] = static_cast<float>(ypos) - _mousePosition[1];
		_mousePosition[0] = static_cast<float>(xpos);
		_mousePosition[1] = static_cast<float>(ypos);
	}

	inline static void getDelta(float& xdelta, float& ydelta) {
		xdelta = _delta[0];
		ydelta = _delta[1];
	}

	inline static void getPosition(float& xpos, float ypos) {
		xpos = _mousePosition[0];
		ypos = _mousePosition[1];
	}

//private:
	inline static constinit bool _mouseDown[5]{};
	inline static constinit bool _mouseDownPrev[5]{};
	inline static constinit float _mousePosition[2]{};
	inline static constinit float _delta[2]{};
};