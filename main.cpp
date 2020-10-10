#include "imgui.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <bitset>
#include "console.hpp"

#include "RenderSystem.hpp"
#include "Image.hpp"

#include "gui.hpp"

std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

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


namespace {
	int get_image_count_from_present_mode(vk::PresentModeKHR present_mode) {
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

	vk::Extent2D select_surface_extent(vk::Extent2D extent, const vk::SurfaceCapabilitiesKHR &surface_capabilities) {
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

	vk::SurfaceFormatKHR select_surface_format(span<vk::SurfaceFormatKHR> surface_formats, span<vk::Format> request_formats, vk::ColorSpaceKHR request_color_space) {
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

	vk::PresentModeKHR select_present_mode(span<vk::PresentModeKHR> present_modes, span<vk::PresentModeKHR> request_modes) {
		for (size_t i = 0; i < request_modes.size(); i++)
			for (size_t j = 0; j < present_modes.size(); j++)
				if (request_modes[i] == present_modes[j])
					return request_modes[i];

		return vk::PresentModeKHR::eFifo;
	}
}

struct Application {
	Application() {
		RenderSystem::Get()->initialize();

		glfwSetWindowUserPointer(RenderSystem::Get()->window(), this);
		glfwSetWindowSizeCallback(RenderSystem::Get()->window(), [](GLFWwindow *window, int width, int height) {
			static_cast<Application*>(glfwGetWindowUserPointer(window))->handleWindowResize(width, height);
		});
		glfwSetFramebufferSizeCallback(RenderSystem::Get()->window(), [](GLFWwindow *window, int width, int height) {
			static_cast<Application*>(glfwGetWindowUserPointer(window))->handleFramebufferResize(width, height);
		});
		glfwSetWindowIconifyCallback(RenderSystem::Get()->window(), [](GLFWwindow *window, int iconified) {
			static_cast<Application*>(glfwGetWindowUserPointer(window))->handleIconify(iconified);
		});
		glfwSetKeyCallback(RenderSystem::Get()->window(), [](GLFWwindow *window, int key, int scancode, int action, int mods) {
			static_cast<Application *>(glfwGetWindowUserPointer(window))->keyCallback(key, scancode, action, mods);
		});
		glfwSetMouseButtonCallback(RenderSystem::Get()->window(), [](GLFWwindow *window, int mouseButton, int action, int mods) {
			static_cast<Application *>(glfwGetWindowUserPointer(window))->mouseButtonCallback(mouseButton, action, mods);
		});
		glfwSetCursorPosCallback(RenderSystem::Get()->window(), [](GLFWwindow *window, double xpos, double ypos) {
			static_cast<Application *>(glfwGetWindowUserPointer(window))->cursorPosCallback(xpos, ypos);
		});
		glfwSetScrollCallback(RenderSystem::Get()->window(), [](GLFWwindow *window, double xoffset, double yoffset) {
			static_cast<Application *>(glfwGetWindowUserPointer(window))->scrollCallback(xoffset, yoffset);
		});
		glfwSetCharCallback(RenderSystem::Get()->window(), [](GLFWwindow* window, unsigned int c) {
			static_cast<Application *>(glfwGetWindowUserPointer(window))->charCallback(c);
		});

		_depthFormat = RenderSystem::Get()->getSupportedDepthFormat();

		createSwapchain();
		prepare();

		_gui.initialize(_renderPass, _frameCount);
	}

	void handleWindowResize(int width, int height) {
	}

	void handleFramebufferResize(int width, int height) {
	}

	void handleIconify(int iconified) {

	}

	void keyCallback(int key, int scancode, int action, int mods) {
		_gui.keyCallback(key, scancode, action, mods);
	}

	void mouseButtonCallback(int mouseButton, int action, int mods) {
		_gui.mouseButtonCallback(mouseButton, action, mods);
	}

	void cursorPosCallback(double xpos, double ypos) {
//		_renderGUI.cursorPosCallback(xpos, ypos);
	}

	void scrollCallback(double xoffset, double yoffset) {
		_gui.scrollCallback(xoffset, yoffset);
	}

	void charCallback(unsigned int c) {
		_gui.charCallback(c);
	}

	~Application() {
		RenderSystem::Get()->device().waitIdle();

		_gui.terminate();

		for (uint32_t i = 0; i < _frameCount; i++) {
			RenderSystem::Get()->device().destroyFramebuffer(_framebuffers[i], nullptr);
			RenderSystem::Get()->device().destroyImageView(_swapchainImageViews[i], nullptr);
			RenderSystem::Get()->device().destroyImageView(_depthImageViews[i]);
			_depthImages[i].destroy();

			RenderSystem::Get()->device().freeCommandBuffers(_commandPools[i], 1, &_commandBuffers[i]);
			RenderSystem::Get()->device().destroyCommandPool(_commandPools[i], nullptr);

			RenderSystem::Get()->device().destroyFence(_fences[i], nullptr);
			RenderSystem::Get()->device().destroySemaphore(_imageAcquiredSemaphore[i]);
			RenderSystem::Get()->device().destroySemaphore(_renderCompleteSemaphore[i]);
		}

		RenderSystem::Get()->device().destroyRenderPass(_renderPass, nullptr);
		RenderSystem::Get()->device().destroySwapchainKHR(_swapchain, nullptr);
		RenderSystem::Get()->terminate();
	}

	void prepare() {
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
		_renderPass = RenderSystem::Get()->device().createRenderPass(render_pass_create_info, nullptr);

		vk::ImageCreateInfo depth_image_create_info{
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

		_frameCount = _swapchainImages.size();

		_swapchainImageViews.resize(_frameCount);

		_depthImages.resize(_frameCount);
		_depthImageViews.resize(_frameCount);

		_framebuffers.resize(_frameCount);

		_commandPools.resize(_frameCount);
		_commandBuffers.resize(_frameCount);

		_fences.resize(_frameCount);
		_imageAcquiredSemaphore.resize(_frameCount);
		_renderCompleteSemaphore.resize(_frameCount);

		for (uint32_t i = 0; i < _frameCount; i++) {
			vk::ImageViewCreateInfo swapchainImageViewCreateInfo {
				.image = _swapchainImages[i],
				.viewType = vk::ImageViewType::e2D,
				.format = _surface_format.format,
				.subresourceRange = {
					vk::ImageAspectFlagBits::eColor,
					0, 1, 0, 1
				}
			};

			_swapchainImageViews[i] = RenderSystem::Get()->device().createImageView(swapchainImageViewCreateInfo, nullptr);
			_depthImages[i] = Image::create(depth_image_create_info, {.usage = VMA_MEMORY_USAGE_GPU_ONLY});

			vk::ImageViewCreateInfo depthImageViewCreateInfo{
					.image = _depthImages[i],
					.viewType = vk::ImageViewType::e2D,
					.format = _depthFormat,
					.subresourceRange = {
							vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1
					}
			};

			_depthImageViews[i] = RenderSystem::Get()->device().createImageView(depthImageViewCreateInfo, nullptr);

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

			_framebuffers[i] = RenderSystem::Get()->device().createFramebuffer(framebuffer_create_info, nullptr);
			_commandPools[i] = vkx::createCommandPool(RenderSystem::Get()->graphicsFamily(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
			_commandBuffers[i] = vkx::allocate(_commandPools[i], vk::CommandBufferLevel::ePrimary);
			_fences[i] = RenderSystem::Get()->device().createFence({.flags = vk::FenceCreateFlagBits::eSignaled});

			_imageAcquiredSemaphore[i] = RenderSystem::Get()->device().createSemaphore({}, nullptr);
			_renderCompleteSemaphore[i] = RenderSystem::Get()->device().createSemaphore({}, nullptr);
		}
	}

	void createSwapchain() {
		auto capabilities = RenderSystem::Get()->physicalDevice().getSurfaceCapabilitiesKHR(RenderSystem::Get()->surface());
		auto surfaceFormats = RenderSystem::Get()->physicalDevice().getSurfaceFormatsKHR(RenderSystem::Get()->surface());
		auto presentModes = RenderSystem::Get()->physicalDevice().getSurfacePresentModesKHR(RenderSystem::Get()->surface());

		vk::Format request_formats[] {
				vk::Format::eB8G8R8A8Unorm,
				vk::Format::eR8G8B8A8Unorm,
				vk::Format::eB8G8R8Unorm,
				vk::Format::eR8G8B8Unorm
		};

		vk::PresentModeKHR request_modes [] {
			vk::PresentModeKHR::eFifo
		};

		_surface_extent = select_surface_extent({0, 0}, capabilities);
		_surface_format = select_surface_format(surfaceFormats, request_formats, vk::ColorSpaceKHR::eSrgbNonlinear);
		_present_mode = select_present_mode(presentModes, request_modes);
		int image_count = get_image_count_from_present_mode(_present_mode);

		int min_image_count = image_count;
		if (min_image_count < capabilities.minImageCount) {
			min_image_count = capabilities.minImageCount;
		} else if (capabilities.maxImageCount != 0 && min_image_count > capabilities.maxImageCount) {
			min_image_count = capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo;
		createInfo.surface = RenderSystem::Get()->surface();
		createInfo.minImageCount = min_image_count;
		createInfo.imageFormat = _surface_format.format;
		createInfo.imageColorSpace = _surface_format.colorSpace;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		uint32_t queue_family_indices[] = {
				RenderSystem::Get()->graphicsFamily(),
				RenderSystem::Get()->presentFamily()
		};
		if (RenderSystem::Get()->graphicsFamily() != RenderSystem::Get()->presentFamily()) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queue_family_indices;
		} else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = _present_mode;
		createInfo.clipped = true;
		createInfo.oldSwapchain = nullptr;
		createInfo.imageExtent = _surface_extent;

		_swapchain = RenderSystem::Get()->device().createSwapchainKHR(createInfo, nullptr);
		_swapchainImages = RenderSystem::Get()->device().getSwapchainImagesKHR(_swapchain);
	}

	vk::CommandBuffer begin() {
		constexpr auto timeout = std::numeric_limits<uint64_t>::max();
		auto semaphore = _imageAcquiredSemaphore[_semaphoreIndex];

		RenderSystem::Get()->device().acquireNextImageKHR(_swapchain, timeout, semaphore, nullptr, &_frameIndex);
		RenderSystem::Get()->device().waitForFences(1, &_fences[_frameIndex], true, timeout);
		RenderSystem::Get()->device().resetFences(1, &_fences[_frameIndex]);

		auto cmd = _commandBuffers[_frameIndex];
		cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

		vk::Rect2D render_area{
			.offset = {0, 0},
			.extent = {
				uint32_t(RenderSystem::Get()->width()),
				uint32_t(RenderSystem::Get()->height())
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

		cmd.beginRenderPass(beginInfo, vk::SubpassContents::eInline);

		return cmd;
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

		RenderSystem::Get()->graphicsQueue().submit(1, &submitInfo, _fences[_frameIndex]);

		vk::PresentInfoKHR presentInfo {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &render_complete_semaphore,
			.swapchainCount = 1,
			.pSwapchains = &_swapchain,
			.pImageIndices = &_frameIndex
		};
		RenderSystem::Get()->presentQueue().presentKHR(presentInfo);

		_semaphoreIndex = (_semaphoreIndex + 1) % _frameCount;
	}

	void run() {
		Console console;

		while (!glfwWindowShouldClose(RenderSystem::Get()->window())) {
			glfwPollEvents();

			_gui.begin();
			console.Draw();
			_gui.end();

			auto cmd = begin();
			_gui.draw(cmd);
			end();
		}
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
	std::vector<vk::CommandPool> _commandPools;
	std::vector<vk::CommandBuffer> _commandBuffers;
	std::vector<vk::Framebuffer> _framebuffers;
	std::vector<vk::Fence> _fences;
};

int main(int, char**) {
	Application app{};
	app.run();
    return 0;
}