#include "imgui.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <iostream>
#include <bitset>
#include <ResourcePack.hpp>
#include "console.hpp"

#include "RenderSystem.hpp"
#include "Image.hpp"

#include "gui.hpp"
#include "WindowGLFW.hpp"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;
};

Vertex vertices[] {
		Vertex{glm::vec3(-0.5,  0.5, 0.5), glm::vec3(0.0, 0.0, 0.0), glm::vec2(0, 1)},
		Vertex{glm::vec3( 0.5,  0.5, 0.5), glm::vec3(1.0, 0.0, 0.0), glm::vec2(1, 1)},
		Vertex{glm::vec3(-0.5, -0.5, 0.5), glm::vec3(0.0, 1.0, 1.0), glm::vec2(0, 0)},
		Vertex{glm::vec3( 0.5, -0.5, 0.5), glm::vec3(1.0, 1.0, 1.0), glm::vec2(1, 0)},

		Vertex{glm::vec3(0.5, -0.5,  0.5), glm::vec3(0.0, 0.0, 0.0), glm::vec2(0, 0)},
		Vertex{glm::vec3(0.5, -0.5, -0.5), glm::vec3(1.0, 0.0, 0.0), glm::vec2(1, 0)},
		Vertex{glm::vec3(0.5,  0.5,  0.5), glm::vec3(0.0, 1.0, 1.0), glm::vec2(0, 1)},
		Vertex{glm::vec3(0.5,  0.5, -0.5), glm::vec3(1.0, 1.0, 1.0), glm::vec2(1, 1)},
};

uint32_t indices[] {
	0, 1, 2, 2, 1, 3,
	4, 5, 6, 6, 5, 7
};

struct CommandPool {
	vk::CommandPool _commandPool;

	inline static CommandPool create(uint32_t queue_index, vk::CommandPoolCreateFlags flags) {
		vk::CommandPoolCreateInfo createInfo;
		createInfo.flags = flags;
		createInfo.queueFamilyIndex = queue_index;
		return {RenderSystem::Get()->device().createCommandPool(createInfo, nullptr)};
	}

	void destroy() {
		RenderSystem::Get()->device().destroyCommandPool(_commandPool);
	}

	inline vk::CommandBuffer allocate(vk::CommandBufferLevel level) {
		vk::CommandBufferAllocateInfo allocateInfo {
			.commandPool = _commandPool,
			.level = level,
			.commandBufferCount = 1
		};

		vk::CommandBuffer commandBuffer;
		RenderSystem::Get()->device().allocateCommandBuffers(&allocateInfo, &commandBuffer);
		return commandBuffer;
	}

	void free(vk::CommandBuffer commandBuffer) {
		RenderSystem::Get()->device().freeCommandBuffers(_commandPool, 1, &commandBuffer);
	}
};

struct Application {
	WindowGLFW _window;
	RenderSystem* Core;
	Console _console{};
	ResourcePack resourcePack{"assets"};

	Application() {
		_window.create(1280, 720, "Vulkan");

		Core = RenderSystem::Get();
		Core->initialize(_window._window);

		glfwSetWindowUserPointer(_window._window, this);
		glfwSetWindowSizeCallback(_window._window, [](GLFWwindow *window, int width, int height) {
			static_cast<Application*>(glfwGetWindowUserPointer(window))->handleWindowResize(width, height);
		});
		glfwSetFramebufferSizeCallback(_window._window, [](GLFWwindow *window, int width, int height) {
			static_cast<Application*>(glfwGetWindowUserPointer(window))->handleFramebufferResize(width, height);
		});
		glfwSetWindowIconifyCallback(_window._window, [](GLFWwindow *window, int iconified) {
			static_cast<Application*>(glfwGetWindowUserPointer(window))->handleIconify(iconified);
		});
		glfwSetKeyCallback(_window._window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
			static_cast<Application *>(glfwGetWindowUserPointer(window))->keyCallback(key, scancode, action, mods);
		});
		glfwSetMouseButtonCallback(_window._window, [](GLFWwindow *window, int mouseButton, int action, int mods) {
			static_cast<Application *>(glfwGetWindowUserPointer(window))->mouseButtonCallback(mouseButton, action, mods);
		});
		glfwSetCursorPosCallback(_window._window, [](GLFWwindow *window, double xpos, double ypos) {
			static_cast<Application *>(glfwGetWindowUserPointer(window))->cursorPosCallback(xpos, ypos);
		});
		glfwSetScrollCallback(_window._window, [](GLFWwindow *window, double xoffset, double yoffset) {
			static_cast<Application *>(glfwGetWindowUserPointer(window))->scrollCallback(xoffset, yoffset);
		});
		glfwSetCharCallback(_window._window, [](GLFWwindow* window, unsigned int c) {
			static_cast<Application *>(glfwGetWindowUserPointer(window))->charCallback(c);
		});

		_depthFormat = Core->getSupportedDepthFormat();

		createSwapchain();
		createRenderPass();
		createSyncObjects();
		createFrameObjects();

		_gui.initialize(_window._window, _renderPass, _frameCount);
	}

	~Application() {
		Core->device().waitIdle();

		_gui.terminate();

		for (uint32_t i = 0; i < _frameCount; i++) {
			Core->device().destroyFramebuffer(_framebuffers[i], nullptr);
			Core->device().destroyImageView(_swapchainImageViews[i], nullptr);
			Core->device().destroyImageView(_depthImageViews[i]);
			_depthImages[i].destroy();

			_commandPools[i].free(_commandBuffers[i]);
			_commandPools[i].destroy();

			Core->device().destroyFence(_fences[i], nullptr);
			Core->device().destroySemaphore(_imageAcquiredSemaphore[i]);
			Core->device().destroySemaphore(_renderCompleteSemaphore[i]);
		}

		Core->device().destroyRenderPass(_renderPass, nullptr);
		Core->device().destroySwapchainKHR(_swapchain, nullptr);
		Core->terminate();

		_window.destroy();
	}

	void run() {
		while (!glfwWindowShouldClose(_window._window)) {
			glfwPollEvents();

			_gui.begin();
			_console.Draw();
			_gui.end();

			auto cmd = begin();
			_gui.draw(cmd);
			end();
		}
	}

private:
	inline static int getImageCountFromPresentMode(vk::PresentModeKHR present_mode) {
		switch (present_mode) {
		case vk::PresentModeKHR::eImmediate:
			return 1;
		case vk::PresentModeKHR::eMailbox:
			return 3;
		case vk::PresentModeKHR::eFifo:
		case vk::PresentModeKHR::eFifoRelaxed:
			return 2;
//		case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
//			break;
//		case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
//			break;
		default:
			return 1;
		}
	}

	inline static vk::Extent2D selectSurfaceExtent(vk::Extent2D extent, const vk::SurfaceCapabilitiesKHR &surface_capabilities) {
		if (surface_capabilities.currentExtent.width != UINT32_MAX) {
			return surface_capabilities.currentExtent;
		}

		auto minExtent = surface_capabilities.minImageExtent;
		auto maxExtent = surface_capabilities.maxImageExtent;

		return {
				std::clamp(extent.width, minExtent.width, maxExtent.width),
				std::clamp(extent.height, minExtent.height, maxExtent.height)
		};
	}

	inline static vk::SurfaceFormatKHR selectSurfaceFormat(span<vk::SurfaceFormatKHR> surface_formats, span<vk::Format> request_formats, vk::ColorSpaceKHR request_color_space) {
		if (surface_formats.size() == 1) {
			if (surface_formats[0].format == vk::Format::eUndefined) {
				vk::SurfaceFormatKHR ret;
				ret.format = request_formats.front();
				ret.colorSpace = request_color_space;
				return ret;
			}
			return surface_formats[0];
		}

		for (size_t i = 0; i < request_formats.size(); i++)
			for (size_t k = 0; k < surface_formats.size(); k++)
				if (surface_formats[k].format == request_formats[i] &&
						surface_formats[k].colorSpace == request_color_space)
					return surface_formats[k];

		return surface_formats.front();
	}

	inline static vk::PresentModeKHR selectPresentMode(span<vk::PresentModeKHR> present_modes, span<vk::PresentModeKHR> request_modes) {
		for (size_t i = 0; i < request_modes.size(); i++)
			for (size_t j = 0; j < present_modes.size(); j++)
				if (request_modes[i] == present_modes[j])
					return request_modes[i];
		return vk::PresentModeKHR::eFifo;
	}

	inline void createSwapchain() {
		auto capabilities = Core->physicalDevice().getSurfaceCapabilitiesKHR(Core->surface());
		auto surfaceFormats = Core->physicalDevice().getSurfaceFormatsKHR(Core->surface());
		auto presentModes = Core->physicalDevice().getSurfacePresentModesKHR(Core->surface());

		vk::Format request_formats[] {
				vk::Format::eB8G8R8A8Unorm,
				vk::Format::eR8G8B8A8Unorm,
				vk::Format::eB8G8R8Unorm,
				vk::Format::eR8G8B8Unorm
		};

		vk::PresentModeKHR request_modes [] {
			vk::PresentModeKHR::eFifo
		};

		_surface_extent = selectSurfaceExtent({0, 0}, capabilities);
		_surface_format = selectSurfaceFormat(surfaceFormats, request_formats, vk::ColorSpaceKHR::eSrgbNonlinear);
		_present_mode = selectPresentMode(presentModes, request_modes);
		int image_count = getImageCountFromPresentMode(_present_mode);

		int min_image_count = image_count;
		if (min_image_count < capabilities.minImageCount) {
			min_image_count = capabilities.minImageCount;
		} else if (capabilities.maxImageCount != 0 && min_image_count > capabilities.maxImageCount) {
			min_image_count = capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR swapchainCreateInfo {
			.surface = Core->surface(),
			.minImageCount = static_cast<uint32_t>(min_image_count),
			.imageFormat = _surface_format.format,
			.imageColorSpace = _surface_format.colorSpace,
			.imageExtent = _surface_extent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = _present_mode,
			.clipped = true,
			.oldSwapchain = nullptr
		};

		uint32_t queue_family_indices[] = {
				Core->graphicsFamily(),
				Core->presentFamily()
		};

		if (Core->graphicsFamily() != Core->presentFamily()) {
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queue_family_indices;
		}

		_swapchain = Core->device().createSwapchainKHR(swapchainCreateInfo, nullptr);
		_swapchainImages = Core->device().getSwapchainImagesKHR(_swapchain);
		_frameCount = _swapchainImages.size();
	}

	void createRenderPass() {
		vk::AttachmentDescription attachments[]{
				vk::AttachmentDescription{
						{},
						_surface_format.format,
						vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear,
						vk::AttachmentStoreOp::eStore,
						vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare,
						vk::ImageLayout::eUndefined,
						vk::ImageLayout::ePresentSrcKHR
				},
				vk::AttachmentDescription{
						{},
						_depthFormat,
						vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear,
						vk::AttachmentStoreOp::eStore,
						vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare,
						vk::ImageLayout::eUndefined,
						vk::ImageLayout::eDepthStencilAttachmentOptimal
				}
		};

		vk::AttachmentReference color_attachment{
				0, vk::ImageLayout::eColorAttachmentOptimal
		};

		vk::AttachmentReference depth_attachment{
				1, vk::ImageLayout::eDepthStencilAttachmentOptimal
		};

		vk::SubpassDescription subpass{
				{},
				vk::PipelineBindPoint::eGraphics,
				0,
				nullptr,
				1,
				&color_attachment,
				nullptr,
				&depth_attachment,
				0,
				nullptr
		};

		vk::SubpassDependency dependency{
				VK_SUBPASS_EXTERNAL, 0,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				{},
				vk::AccessFlagBits::eColorAttachmentWrite
		};

		vk::RenderPassCreateInfo render_pass_create_info{
				.attachmentCount = 2,
				.pAttachments = attachments,
				.subpassCount = 1,
				.pSubpasses = &subpass,
				.dependencyCount = 1,
				.pDependencies = &dependency,
		};
		_renderPass = Core->device().createRenderPass(render_pass_create_info, nullptr);
	}

	void createSyncObjects() {
		_fences.resize(_frameCount);
		_imageAcquiredSemaphore.resize(_frameCount);
		_renderCompleteSemaphore.resize(_frameCount);

		for (uint32_t i = 0; i < _frameCount; i++) {
			_fences[i] = Core->device().createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
			_imageAcquiredSemaphore[i] = Core->device().createSemaphore({}, nullptr);
			_renderCompleteSemaphore[i] = Core->device().createSemaphore({}, nullptr);
		}
	}

	void createFrameObjects() {
		_commandPools.resize(_frameCount);
		_commandBuffers.resize(_frameCount);
		_framebuffers.resize(_frameCount);
		_depthImages.resize(_frameCount);
		_depthImageViews.resize(_frameCount);
		_swapchainImageViews.resize(_frameCount);

		vk::ImageViewCreateInfo swapchainImageViewCreateInfo{
				.viewType = vk::ImageViewType::e2D,
				.format = _surface_format.format,
				.subresourceRange = {
						vk::ImageAspectFlagBits::eColor,
						0, 1, 0, 1
				}
		};
		vk::ImageCreateInfo depthImageCreateInfo{
				.imageType = vk::ImageType::e2D,
				.format = _depthFormat,
				.extent = vk::Extent3D{
						_surface_extent.width,
						_surface_extent.height,
						1
				},
				.mipLevels = 1,
				.arrayLayers = 1,
				.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
		};
		vk::ImageViewCreateInfo depthImageViewCreateInfo{
				.viewType = vk::ImageViewType::e2D,
				.format = _depthFormat,
				.subresourceRange = {
						vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1
				}
		};
		for (uint32_t i = 0; i < _frameCount; i++) {
			swapchainImageViewCreateInfo.image = _swapchainImages[i];
			_swapchainImageViews[i] = Core->device().createImageView(swapchainImageViewCreateInfo, nullptr);

			_depthImages[i] = Image::create(depthImageCreateInfo, {.usage = VMA_MEMORY_USAGE_GPU_ONLY});

			depthImageViewCreateInfo.image = _depthImages[i];
			_depthImageViews[i] = Core->device().createImageView(depthImageViewCreateInfo, nullptr);

			vk::ImageView attachments[]{
					_swapchainImageViews[i],
					_depthImageViews[i]
			};

			vk::FramebufferCreateInfo framebuffer_create_info{
					.renderPass = _renderPass,
					.attachmentCount = 2,
					.pAttachments = attachments,
					.width = _surface_extent.width,
					.height = _surface_extent.height,
					.layers = 1
			};

			_framebuffers[i] = Core->device().createFramebuffer(framebuffer_create_info, nullptr);
			_commandPools[i] = CommandPool::create(Core->graphicsFamily(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
			_commandBuffers[i] = _commandPools[i].allocate(vk::CommandBufferLevel::ePrimary);
		}
	}

	vk::CommandBuffer begin() {
		static constinit auto timeout = std::numeric_limits<uint64_t>::max();
		auto semaphore = _imageAcquiredSemaphore[_semaphoreIndex];

		Core->device().acquireNextImageKHR(_swapchain, timeout, semaphore, nullptr, &_frameIndex);
		Core->device().waitForFences(1, &_fences[_frameIndex], true, timeout);
		Core->device().resetFences(1, &_fences[_frameIndex]);

		_commandBuffers[_frameIndex].begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

		vk::Rect2D render_area{
			.offset = {0, 0},
			.extent = {
				uint32_t(_window.width()),
				uint32_t(_window.height())
			}
		};

		vk::ClearValue clearColors[]{
			vk::ClearColorValue(std::array{0.0f, 0.0f, 0.0f, 1.0f}),
			vk::ClearDepthStencilValue{1.0f, 0}
		};

		vk::RenderPassBeginInfo beginInfo {
			.renderPass = _renderPass,
			.framebuffer = _framebuffers[_frameIndex],
			.renderArea = render_area,
			.clearValueCount = 2,
			.pClearValues = clearColors
		};

		_commandBuffers[_frameIndex].beginRenderPass(beginInfo, vk::SubpassContents::eInline);
		return _commandBuffers[_frameIndex];
	}

	void end() {
		_commandBuffers[_frameIndex].endRenderPass();
		_commandBuffers[_frameIndex].end();

		auto image_acquired_semaphore = _imageAcquiredSemaphore[_semaphoreIndex];
		auto render_complete_semaphore = _renderCompleteSemaphore[_semaphoreIndex];

		vk::PipelineStageFlags stages[] = {
			vk::PipelineStageFlagBits::eColorAttachmentOutput
		};

		vk::SubmitInfo submitInfo {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &image_acquired_semaphore,
			.pWaitDstStageMask = stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &_commandBuffers[_frameIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &render_complete_semaphore
		};

		Core->graphicsQueue().submit(1, &submitInfo, _fences[_frameIndex]);

		vk::PresentInfoKHR presentInfo {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &render_complete_semaphore,
			.swapchainCount = 1,
			.pSwapchains = &_swapchain,
			.pImageIndices = &_frameIndex
		};
		Core->presentQueue().presentKHR(presentInfo);

		_semaphoreIndex = (_semaphoreIndex + 1) % _frameCount;
	}

private:
	void handleWindowResize(int width, int height) {
	}

	void handleFramebufferResize(int width, int height) {}

	void handleIconify(int iconified) {

	}

	void keyCallback(int key, int scancode, int action, int mods) {
		_gui.keyCallback(key, scancode, action, mods);
	}

	void mouseButtonCallback(int mouseButton, int action, int mods) {
		_gui.mouseButtonCallback(mouseButton, action, mods);
	}

	void cursorPosCallback(double xpos, double ypos) {
	}

	void scrollCallback(double xoffset, double yoffset) {
		_gui.scrollCallback(xoffset, yoffset);
	}

	void charCallback(unsigned int c) {
		_gui.charCallback(c);
	}

private:
	GUI _gui;

	std::vector<Image> _depthImages;
	std::vector<vk::ImageView> _depthImageViews;

	std::vector<vk::Semaphore> _imageAcquiredSemaphore;
	std::vector<vk::Semaphore> _renderCompleteSemaphore;

	vk::RenderPass _renderPass;
	vk::Format _depthFormat;

	uint32_t _frameIndex = 0;
	uint32_t _frameCount = 0;
	uint32_t _semaphoreIndex = 0;

	vk::Extent2D _surface_extent;
	vk::SurfaceFormatKHR _surface_format;
	vk::PresentModeKHR _present_mode;

	vk::SwapchainKHR _swapchain;
	std::vector<vk::Image> _swapchainImages;
	std::vector<vk::ImageView> _swapchainImageViews;
	std::vector<CommandPool> _commandPools;
	std::vector<vk::CommandBuffer> _commandBuffers;
	std::vector<vk::Framebuffer> _framebuffers;
	std::vector<vk::Fence> _fences;
};

int main(int, char**) {
	Application app{};
	app.run();
    return 0;
}