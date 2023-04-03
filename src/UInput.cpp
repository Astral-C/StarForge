#include "UInput.hpp"
#include <GLFW/glfw3.h>

namespace UInput {
	// Anonymous namespace within UInput allows us to have private-scope variables that are only accessible from this TU.
	// This allows us to hide direct access to the input state variables and only allow access via the Get functions.
	namespace {
		constexpr uint32_t KEY_MAX = 512;
		constexpr uint32_t MOUSE_BUTTON_MAX = 3;

		glm::vec2 mMousePosition;
		glm::vec2 mMouseDelta;
		int32_t mMouseScrollDelta;

		bool mKeysDown[KEY_MAX];
		bool mPrevKeysDown[KEY_MAX];

		bool mMouseButtonsDown[MOUSE_BUTTON_MAX];
		bool mPrevMouseButtonsDown[MOUSE_BUTTON_MAX];
		glm::vec2 mPrevMousePosition;

		void SetKeyboardState(uint32_t key, bool pressed) {
			mKeysDown[key] = pressed;
		}

		void SetMouseState(uint32_t button, bool pressed) {
			mMouseButtonsDown[button] = pressed;
		}

		void SetMousePosition(uint32_t x, uint32_t y) {
			mMousePosition = glm::vec2(x, y);
		}

		void SetMouseScrollDelta(uint32_t delta) {
			mMouseScrollDelta = delta;
		}
	}
}

bool UInput::GetKey(uint32_t key) {
	return mKeysDown[key];
}

bool UInput::GetKeyDown(uint32_t key) {
	return mKeysDown[key] && !mPrevKeysDown[key];
}

bool UInput::GetKeyUp(uint32_t key) {
	return mPrevKeysDown[key] && !mKeysDown[key];
}

bool UInput::GetMouseButton(uint32_t button) {
	return mMouseButtonsDown[button];
}

bool UInput::GetMouseButtonDown(uint32_t button) {
	return mMouseButtonsDown[button] && !mPrevMouseButtonsDown[button];
}

bool UInput::GetMouseButtonUp(uint32_t button) {
	return mPrevMouseButtonsDown[button] && !mMouseButtonsDown[button];
}

glm::vec2 UInput::GetMousePosition() {
	return mMousePosition;
}

glm::vec2 UInput::GetMouseDelta() {
	return mMouseDelta;
}

int32_t UInput::GetMouseScrollDelta() {
	return mMouseScrollDelta;
}

void UInput::UpdateInputState() {
 for (int i = 0; i < KEY_MAX; i++)
	 mPrevKeysDown[i] = mKeysDown[i];
 for (int i = 0; i < MOUSE_BUTTON_MAX; i++)
	 mPrevMouseButtonsDown[i] = mMouseButtonsDown[i];

 mMouseDelta = mMousePosition - mPrevMousePosition;
 mPrevMousePosition = mMousePosition;
 mMouseScrollDelta = 0;
}

void UInput::GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key >= KEY_MAX)
		return;

	if (action == GLFW_PRESS)
		SetKeyboardState(key, true);
	else if (action == GLFW_RELEASE)
		SetKeyboardState(key, false);
}

void UInput::GLFWMousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
	SetMousePosition(xpos, ypos);
}

void UInput::GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button >= MOUSE_BUTTON_MAX)
		return;

	if (action == GLFW_PRESS)
		SetMouseState(button, true);
	else if (action == GLFW_RELEASE)
		SetMouseState(button, false);
}

void UInput::GLFWMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	SetMouseScrollDelta(yoffset);
}
