#pragma once

#include "RenderSystem.hpp"

#include "client/util/DescriptorPool.hpp"
#include "client/util/CommandPool.hpp"

#include <glm/mat4x4.hpp>

struct RenderTexture {
	VkImage image;
	VkImageView view;
	VkSampler sampler;
	VmaAllocation allocation;
};

struct CameraTransform {
	glm::mat4 camera;
};

struct RenderContext {
	RenderSystem* core = RenderSystem::Instance();

	CommandPool commandPool;
	DescriptorPool descriptorPool;

	RenderContext();
	~RenderContext();

private:
	static int _getImageCountFromPresentMode(vk::PresentModeKHR present_mode);
	static vk::Extent2D _selectSurfaceExtent(vk::Extent2D extent, const vk::SurfaceCapabilitiesKHR &surface_capabilities);
	static vk::SurfaceFormatKHR _selectSurfaceFormat(std::span<const vk::SurfaceFormatKHR> surface_formats, std::span<const vk::Format> request_formats, vk::ColorSpaceKHR request_color_space);
	static vk::PresentModeKHR _selectPresentMode(std::span<const vk::PresentModeKHR> present_modes, std::span<const vk::PresentModeKHR> request_modes);

	void _createSwapchain();
	void _createRenderPass();
	void _createSyncObjects();
	void _createFrameObjects();

public:
	vk::CommandBuffer begin();
	void end();

	RenderTexture* createTexture2D(vk::Format format, uint32_t width, uint32_t height);
	RenderTexture* createDepthTexture(vk::Format format, uint32_t width, uint32_t height);
	void textureSubImage2D(RenderTexture* texture, uint32_t width, uint32_t height, int channels, const void* pixels);

public:
	void setRenderSize(int width, int height) {
		renderArea.extent.width = width;
		renderArea.extent.height = height;
	}

//private:

	uint32_t frameIndex = 0;
	uint32_t frameCount = 0;
	uint32_t semaphoreIndex = 0;

	vk::Format depthFormat;

	vk::Rect2D renderArea;
	vk::RenderPass renderPass;

	vk::Extent2D surfaceExtent;
	vk::SurfaceFormatKHR surfaceFormat;
	vk::PresentModeKHR presentMode;

	vk::SwapchainKHR swapchain;

// frame objects

	std::vector<vk::Fence> fences;
	std::vector<vk::Semaphore> imageAcquiredSemaphore;
	std::vector<vk::Semaphore> renderCompleteSemaphore;

	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> swapchainImageViews;

	std::vector<RenderTexture*> depthTextures;
	std::vector<vk::Framebuffer> framebuffers;

	std::vector<CommandPool> commandPools;
	std::vector<vk::CommandBuffer> commandBuffers;
};