#include "RenderContext.hpp"

#include "Buffer.hpp"

RenderContext::RenderContext() {
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

	_createSwapchain();
	_createRenderPass();
	_createSyncObjects();
	_createFrameObjects();
}

RenderContext::~RenderContext() {
	core->device().waitIdle();

	for (uint32_t i = 0; i < frameCount; i++) {
		core->device().destroyFramebuffer(framebuffers[i], nullptr);
		core->device().destroyImageView(swapchainImageViews[i], nullptr);

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

int RenderContext::_getImageCountFromPresentMode(vk::PresentModeKHR present_mode) {
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

vk::Extent2D RenderContext::_selectSurfaceExtent(vk::Extent2D extent, const vk::SurfaceCapabilitiesKHR &surface_capabilities) {
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

vk::SurfaceFormatKHR RenderContext::_selectSurfaceFormat(std::span<const vk::SurfaceFormatKHR> surface_formats, std::span<const vk::Format> request_formats, vk::ColorSpaceKHR request_color_space) {
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

vk::PresentModeKHR RenderContext::_selectPresentMode(std::span<const vk::PresentModeKHR> present_modes, std::span<const vk::PresentModeKHR> request_modes) {
	for (size_t i = 0; i < request_modes.size(); i++)
		for (size_t j = 0; j < present_modes.size(); j++)
			if (request_modes[i] == present_modes[j])
				return request_modes[i];
	return vk::PresentModeKHR::eFifo;
}

void RenderContext::_createSwapchain() {
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

	surfaceExtent = _selectSurfaceExtent({0, 0}, capabilities);
	surfaceFormat = _selectSurfaceFormat(surfaceFormats, request_formats, vk::ColorSpaceKHR::eSrgbNonlinear);
	presentMode = _selectPresentMode(presentModes, request_modes);
	int image_count = _getImageCountFromPresentMode(presentMode);

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

void RenderContext::_createRenderPass() {
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

void RenderContext::_createSyncObjects() {
	fences.resize(frameCount);
	imageAcquiredSemaphore.resize(frameCount);
	renderCompleteSemaphore.resize(frameCount);

	for (uint32_t i = 0; i < frameCount; i++) {
		fences[i] = core->device().createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
		imageAcquiredSemaphore[i] = core->device().createSemaphore({}, nullptr);
		renderCompleteSemaphore[i] = core->device().createSemaphore({}, nullptr);
	}
}

void RenderContext::_createFrameObjects() {
	commandPools.resize(frameCount);
	commandBuffers.resize(frameCount);
	framebuffers.resize(frameCount);
	depthTextures.resize(frameCount);
	swapchainImageViews.resize(frameCount);

	vk::ImageViewCreateInfo swapchainImageViewCreateInfo{
			.viewType = vk::ImageViewType::e2D,
			.format = surfaceFormat.format,
			.subresourceRange = {
					vk::ImageAspectFlagBits::eColor,
					0, 1, 0, 1
			}
	};
	for (uint32_t i = 0; i < frameCount; i++) {
		depthTextures[i] = createDepthTexture(depthFormat, surfaceExtent.width, surfaceExtent.height);

		swapchainImageViewCreateInfo.image = swapchainImages[i];
		swapchainImageViews[i] = core->device().createImageView(swapchainImageViewCreateInfo, nullptr);

		vk::ImageView attachments[]{ swapchainImageViews[i], depthTextures[i]->view };

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

vk::CommandBuffer RenderContext::begin() {
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

void RenderContext::end() {
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
//	core->presentQueue().waitIdle();

	semaphoreIndex = (semaphoreIndex + 1) % frameCount;
}

RenderTexture* RenderContext::createTexture2D(vk::Format format, uint32_t width, uint32_t height) {
	 VkImageCreateInfo imageCreateInfo = vk::ImageCreateInfo{
		.imageType = vk::ImageType::e2D,
		.format = format,
		.extent = {
				.width = width,
				.height = height,
				.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
	};

	auto texture = new RenderTexture();
	VmaAllocationCreateInfo allocationCreateInfo{.usage = VMA_MEMORY_USAGE_GPU_ONLY};
	vmaCreateImage(core->allocator(), &imageCreateInfo, &allocationCreateInfo, &texture->image, &texture->allocation, nullptr);

	vk::ImageViewCreateInfo imageViewCreateInfo {
		.image = texture->image,
		.viewType = vk::ImageViewType::e2D,
		.format = format,
		.subresourceRange{
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	texture->view = core->device().createImageView(imageViewCreateInfo);
	return texture;
}

RenderTexture* RenderContext::createDepthTexture(vk::Format format, uint32_t width, uint32_t height) {
	 VkImageCreateInfo imageCreateInfo = vk::ImageCreateInfo{
		.imageType = vk::ImageType::e2D,
		.format = format,
		.extent = {
			.width = width,
			.height = height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment
	};

	auto texture = new RenderTexture();
	VmaAllocationCreateInfo allocationCreateInfo{.usage = VMA_MEMORY_USAGE_GPU_ONLY};
	vmaCreateImage(core->allocator(), &imageCreateInfo, &allocationCreateInfo, &texture->image, &texture->allocation, nullptr);

	vk::ImageViewCreateInfo imageViewCreateInfo {
		.image = texture->image,
		.viewType = vk::ImageViewType::e2D,
		.format = format,
		.subresourceRange{
			.aspectMask = vk::ImageAspectFlagBits::eDepth,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	texture->view = core->device().createImageView(imageViewCreateInfo);
	return texture;
}

void RenderContext::textureSubImage2D(RenderTexture* texture, uint32_t width, uint32_t height, int channels, const void *pixels) {
	auto cmd = commandPool.allocate(vk::CommandBufferLevel::ePrimary);
	cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

	vk::DeviceSize bufferSize = width * height * channels;

	vk::BufferCreateInfo srcBufferCI{.size = bufferSize, .usage = vk::BufferUsageFlagBits::eTransferSrc};

	auto srcBuffer = Buffer::create(srcBufferCI, {.usage = VMA_MEMORY_USAGE_CPU_ONLY});
	std::memcpy(srcBuffer.map(), pixels, bufferSize);
	srcBuffer.unmap();

	vk::ImageMemoryBarrier copy_barrier{
			.dstAccessMask = vk::AccessFlagBits::eTransferWrite,
			.oldLayout = vk::ImageLayout::eUndefined,
			.newLayout = vk::ImageLayout::eTransferDstOptimal,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture->image,
			.subresourceRange {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.levelCount = 1,
					.layerCount = 1
			}
	};

	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &copy_barrier);

	vk::BufferImageCopy region{
			.imageSubresource {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.layerCount = 1
			},
			.imageExtent = {
					.width = static_cast<uint32_t>(width),
					.height = static_cast<uint32_t>(height),
					.depth = 1
			}
	};

	cmd.copyBufferToImage(srcBuffer, texture->image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

	vk::ImageMemoryBarrier use_barrier{
			.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
			.dstAccessMask = vk::AccessFlagBits::eShaderRead,
			.oldLayout = vk::ImageLayout::eTransferDstOptimal,
			.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture->image,
			.subresourceRange = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.levelCount = 1,
					.layerCount = 1,
			}
	};

	cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &use_barrier);
	cmd.end();

	vk::SubmitInfo submitInfo{
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd
	};
	core->graphicsQueue().submit(1, &submitInfo, nullptr);
	core->graphicsQueue().waitIdle();

	commandPool.free(cmd);

	srcBuffer.destroy();
}