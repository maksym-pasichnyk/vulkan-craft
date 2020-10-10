#pragma once

#include <imgui/imgui.h>

#include "CommandPool.hpp"
#include "Texture.hpp"
#include "RenderBuffer.hpp"

struct GUI {
	void initialize(GLFWwindow* window, vk::RenderPass renderPass, int frameCount);

	void terminate();

	void begin();
	void end();

	void draw(vk::CommandBuffer cmd);

	void mouseButtonCallback(int button, int action, int mods);

	void scrollCallback(double xoffset, double yoffset);

	void keyCallback(int key, int scancode, int action, int mods);

	void charCallback(unsigned int c);

private:
	static const char *GetClipboardText(void *user_data);

	static void SetClipboardText(void *user_data, const char *text);

	void setupRenderState(ImDrawData *draw_data, vk::CommandBuffer cmd, RenderBuffer *rb, int fb_width, int fb_height);

	void createFontsTexture();

	void updateMousePosAndButtons();

	void updateMouseCursor();

	void updateGamepads();

private:
	GLFWwindow* _window;

	vk::DescriptorSetLayout _fontDescriptorSetLayout;
	vk::DescriptorSet _fontDescriptor;

	vk::PipelineLayout _pipelineLayout;
	vk::Pipeline _pipeline;

	vk::Sampler _fontSampler;
	Texture _fontTexture;

	vk::DescriptorPool _descriptorPool;
	vk::CommandPool _commandPool;

	uint32_t _frameIndex;
	uint32_t _frameCount;

	std::vector<RenderBuffer> _renderBuffers;

	double _time = 0.0;
	bool _mouseJustPressed[ImGuiMouseButton_COUNT] = {};
	GLFWcursor *_mouseCursors[ImGuiMouseCursor_COUNT] = {};
};