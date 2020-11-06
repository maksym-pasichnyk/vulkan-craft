#pragma once

#include <imgui/imgui.h>

#include <client/renderer/texture/Texture.hpp>
#include <client/renderer/RenderBuffer.hpp>
#include "client/util/CommandPool.hpp"
#include "client/util/DescriptorPool.hpp"

#include <client/util/Handle.hpp>

struct RenderContext;

struct GUI {
	GUI(GLFWwindow* window, Handle<RenderContext> renderContext);
	~GUI();

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

	void createFontsTexture(Handle<RenderContext> renderContext);

	void updateMousePosAndButtons();

	void updateMouseCursor();

	void updateGamepads();

private:
	RenderSystem* core = RenderSystem::Instance();

	GLFWwindow* _window;

	vk::DescriptorSetLayout _fontDescriptorSetLayout;
	vk::DescriptorSet _fontDescriptor;

	vk::PipelineLayout _pipelineLayout;
	vk::Pipeline _pipeline;

	vk::Sampler _fontSampler;
	RenderTexture* _fontTexture;

	DescriptorPool _descriptorPool;

	uint32_t _frameIndex;
	uint32_t _frameCount;

	std::vector<RenderBuffer> _renderBuffers;

	double _time = 0.0;
	bool _mouseJustPressed[ImGuiMouseButton_COUNT] = {};
	GLFWcursor *_mouseCursors[ImGuiMouseCursor_COUNT] = {};
};