#pragma once

#include "RenderSystem.hpp"

#include "DescriptorPool.hpp"
#include "CommandPool.hpp"
#include "Image.hpp"

struct CameraTransform {
	glm::mat4 camera;
};

struct RenderContext {
	RenderSystem* core = RenderSystem::Instance();

	CommandPool commandPool;
	DescriptorPool descriptorPool;

	RenderContext() {
		vk::DescriptorPoolSize descriptorPoolSizes[] = {
			{vk::DescriptorType::eSampler, 1000},
			{vk::DescriptorType::eCombinedImageSampler, 1000},
			{vk::DescriptorType::eSampledImage, 1000},
			{vk::DescriptorType::eStorageImage, 1000},
			{vk::DescriptorType::eUniformTexelBuffer, 1000},
			{vk::DescriptorType::eStorageTexelBuffer, 1000},
			{vk::DescriptorType::eUniformBuffer, 1000},
			{vk::DescriptorType::eStorageBuffer, 1000},
			{vk::DescriptorType::eUniformBufferDynamic, 1000},
			{vk::DescriptorType::eStorageBufferDynamic, 1000},
			{vk::DescriptorType::eInputAttachment, 1000}
		};

		descriptorPool = DescriptorPool::create(1000, descriptorPoolSizes);
		commandPool = CommandPool::create(core->graphicsFamily(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

		createSwapchain();
		createRenderPass();
		createSyncObjects();
		createFrameObjects();
	}

	~RenderContext() {
		core->device().waitIdle();

		for (uint32_t i = 0; i < frameCount; i++) {
			core->device().destroyFramebuffer(framebuffers[i], nullptr);
			core->device().destroyImageView(swapchainImageViews[i], nullptr);
			core->device().destroyImageView(depthImageViews[i], nullptr);
			depthImages[i].destroy();

			commandPools[i].free(commandBuffers[i]);
			commandPools[i].destroy();

			core->device().destroyFence(fences[i], nullptr);
			core->device().destroySemaphore(imageAcquiredSemaphore[i], nullptr);
			core->device().destroySemaphore(renderCompleteSemaphore[i], nullptr);
		}

		core->device().destroyRenderPass(renderPass, nullptr);
		core->device().destroySwapchainKHR(swapchain, nullptr);
		core->terminate();
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
		auto capabilities = core->physicalDevice().getSurfaceCapabilitiesKHR(core->surface());
		auto surfaceFormats = core->physicalDevice().getSurfaceFormatsKHR(core->surface());
		auto presentModes = core->physicalDevice().getSurfacePresentModesKHR(core->surface());

		vk::Format request_formats[] {
				vk::Format::eB8G8R8A8Unorm,
				vk::Format::eR8G8B8A8Unorm,
				vk::Format::eB8G8R8Unorm,
				vk::Format::eR8G8B8Unorm
		};

		vk::PresentModeKHR request_modes [] {
			vk::PresentModeKHR::eFifo
		};

		surfaceExtent = selectSurfaceExtent({0, 0}, capabilities);
		surfaceFormat = selectSurfaceFormat(surfaceFormats, request_formats, vk::ColorSpaceKHR::eSrgbNonlinear);
		presentMode = selectPresentMode(presentModes, request_modes);
		int image_count = getImageCountFromPresentMode(presentMode);

		int min_image_count = image_count;
		if (min_image_count < capabilities.minImageCount) {
			min_image_count = capabilities.minImageCount;
		} else if (capabilities.maxImageCount != 0 && min_image_count > capabilities.maxImageCount) {
			min_image_count = capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR swapchainCreateInfo {
			.surface = core->surface(),
			.minImageCount = static_cast<uint32_t>(min_image_count),
			.imageFormat = surfaceFormat.format,
			.imageColorSpace = surfaceFormat.colorSpace,
			.imageExtent = surfaceExtent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = presentMode,
			.clipped = true,
			.oldSwapchain = nullptr
		};

		uint32_t queue_family_indices[] = {
				core->graphicsFamily(),
				core->presentFamily()
		};

		if (core->graphicsFamily() != core->presentFamily()) {
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queue_family_indices;
		}

		swapchain = core->device().createSwapchainKHR(swapchainCreateInfo, nullptr);
		swapchainImages = core->device().getSwapchainImagesKHR(swapchain);
		frameCount = swapchainImages.size();
	}

	void createRenderPass() {
		depthFormat = core->getSupportedDepthFormat();

		vk::AttachmentDescription attachments[]{
				vk::AttachmentDescription{
						{},
						surfaceFormat.format,
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
						depthFormat,
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
		renderPass = core->device().createRenderPass(render_pass_create_info, nullptr);
	}

	void createSyncObjects() {
		fences.resize(frameCount);
		imageAcquiredSemaphore.resize(frameCount);
		renderCompleteSemaphore.resize(frameCount);

		for (uint32_t i = 0; i < frameCount; i++) {
			fences[i] = core->device().createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
			imageAcquiredSemaphore[i] = core->device().createSemaphore({}, nullptr);
			renderCompleteSemaphore[i] = core->device().createSemaphore({}, nullptr);
		}
	}

	void createFrameObjects() {
		commandPools.resize(frameCount);
		commandBuffers.resize(frameCount);
		framebuffers.resize(frameCount);
		depthImages.resize(frameCount);
		depthImageViews.resize(frameCount);
		swapchainImageViews.resize(frameCount);

		vk::ImageViewCreateInfo swapchainImageViewCreateInfo{
				.viewType = vk::ImageViewType::e2D,
				.format = surfaceFormat.format,
				.subresourceRange = {
						vk::ImageAspectFlagBits::eColor,
						0, 1, 0, 1
				}
		};
		vk::ImageCreateInfo depthImageCreateInfo{
				.imageType = vk::ImageType::e2D,
				.format = depthFormat,
				.extent = vk::Extent3D{
						surfaceExtent.width,
						surfaceExtent.height,
						1
				},
				.mipLevels = 1,
				.arrayLayers = 1,
				.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
		};
		vk::ImageViewCreateInfo depthImageViewCreateInfo{
				.viewType = vk::ImageViewType::e2D,
				.format = depthFormat,
				.subresourceRange = {
						vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1
				}
		};
		for (uint32_t i = 0; i < frameCount; i++) {
			swapchainImageViewCreateInfo.image = swapchainImages[i];
			swapchainImageViews[i] = core->device().createImageView(swapchainImageViewCreateInfo, nullptr);

			depthImages[i] = Image::create(depthImageCreateInfo, {.usage = VMA_MEMORY_USAGE_GPU_ONLY});

			depthImageViewCreateInfo.image = depthImages[i];
			depthImageViews[i] = core->device().createImageView(depthImageViewCreateInfo, nullptr);

			vk::ImageView attachments[]{
					swapchainImageViews[i],
					depthImageViews[i]
			};

			vk::FramebufferCreateInfo framebuffer_create_info{
					.renderPass = renderPass,
					.attachmentCount = 2,
					.pAttachments = attachments,
					.width = surfaceExtent.width,
					.height = surfaceExtent.height,
					.layers = 1
			};

			framebuffers[i] = core->device().createFramebuffer(framebuffer_create_info, nullptr);
			commandPools[i] = CommandPool::create(core->graphicsFamily(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
			commandBuffers[i] = commandPools[i].allocate(vk::CommandBufferLevel::ePrimary);
		}
	}

public:
	vk::CommandBuffer begin() {
		static constinit auto timeout = std::numeric_limits<uint64_t>::max();
		auto semaphore = imageAcquiredSemaphore[semaphoreIndex];

		core->device().acquireNextImageKHR(swapchain, timeout, semaphore, nullptr, &frameIndex);
		core->device().waitForFences(1, &fences[frameIndex], true, timeout);
		core->device().resetFences(1, &fences[frameIndex]);

		commandBuffers[frameIndex].begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

		vk::ClearValue clearColors[]{
			vk::ClearColorValue(std::array{0, 0, 0, 1}),
			vk::ClearDepthStencilValue{1.0f, 0}
		};

		vk::RenderPassBeginInfo beginInfo {
			.renderPass = renderPass,
			.framebuffer = framebuffers[frameIndex],
			.renderArea = renderArea,
			.clearValueCount = 2,
			.pClearValues = clearColors
		};

		commandBuffers[frameIndex].beginRenderPass(beginInfo, vk::SubpassContents::eInline);
		return commandBuffers[frameIndex];
	}

	void end() {
		commandBuffers[frameIndex].endRenderPass();
		commandBuffers[frameIndex].end();

		auto image_acquired_semaphore = imageAcquiredSemaphore[semaphoreIndex];
		auto render_complete_semaphore = renderCompleteSemaphore[semaphoreIndex];

		vk::PipelineStageFlags stages[] = {
			vk::PipelineStageFlagBits::eColorAttachmentOutput
		};

		vk::SubmitInfo submitInfo {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &image_acquired_semaphore,
			.pWaitDstStageMask = stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffers[frameIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &render_complete_semaphore
		};

		core->graphicsQueue().submit(1, &submitInfo, fences[frameIndex]);

		vk::PresentInfoKHR presentInfo {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &render_complete_semaphore,
			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &frameIndex
		};
		core->presentQueue().presentKHR(presentInfo);

		semaphoreIndex = (semaphoreIndex + 1) % frameCount;
	}

public:
	void setRenderingSize(int width, int height) {
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

	std::vector<Image> depthImages;
	std::vector<vk::ImageView> depthImageViews;

	std::vector<vk::Framebuffer> framebuffers;

	std::vector<CommandPool> commandPools;
	std::vector<vk::CommandBuffer> commandBuffers;
};