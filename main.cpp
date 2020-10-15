#include "imgui.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <ResourceManager.hpp>
#include <glm/ext.hpp>

#include "RenderSystem.hpp"
#include "Image.hpp"

#include "gui.hpp"
#include "GameWindow.hpp"
#include "Clock.hpp"

#include "DescriptorPool.hpp"

#include "Mouse.hpp"
#include "Keyboard.hpp"

#include "Json.hpp"
#include "stb_image.hpp"

#include "VertexBuilder.hpp"

#include "ModelRenderer.hpp"

struct CameraTransform {
	glm::mat4 camera;
};

inline static glm::mat4 PerspectiveProjectionMatrix(float field_of_view, float aspect_ratio, float near_plane, float far_plane) {
	const float tanHalfFovy = glm::tan(field_of_view * 0.5f);

	const float a = 1.0f / (aspect_ratio * tanHalfFovy);
	const float b = -1.0f / tanHalfFovy;
	const float c = far_plane / (far_plane - near_plane);
	const float d = (near_plane * far_plane) / (near_plane - far_plane);

	return {
		a, 0, 0, 0,
		0, b, 0, 0,
		0, 0, c, 1,
		0, 0, d, 0
	};
}

struct GameRenderer {
	RenderSystem* core = RenderSystem::Get();

	DescriptorPool descriptorPool;
	CommandPool commandPool;

	GameRenderer(GameWindow* window) {
		core->initialize(window->getPlatformWindow());

		createSwapchain();
		createRenderPass();
		createSyncObjects();
		createFrameObjects();

		vk::DescriptorPoolSize pool_sizes[] = {
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

		descriptorPool = DescriptorPool::create(1000, pool_sizes);
		commandPool = CommandPool::create(core->graphicsFamily(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	}

	~GameRenderer() {
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
			vk::ClearColorValue(std::array{0.0f, 0.0f, 0.0f, 1.0f}),
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

struct AgentEntity {
	AgentEntity(ResourceManager* resourceManager, GameRenderer* renderer) {
		vk::SamplerCreateInfo samplerCreateInfo{
				.magFilter = vk::Filter::eNearest,
				.minFilter = vk::Filter::eNearest,
				.mipmapMode = vk::SamplerMipmapMode::eNearest,
				.addressModeU = vk::SamplerAddressMode::eRepeat,
				.addressModeV = vk::SamplerAddressMode::eRepeat,
				.addressModeW = vk::SamplerAddressMode::eRepeat,
				.maxAnisotropy = 0.0f,
				.minLod = 0,
				.maxLod = 0
		};

		sampler = core->device().createSampler(samplerCreateInfo, nullptr);

		vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding{
				.binding = 0,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eFragment,
				.pImmutableSamplers = &sampler
		};

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
				.bindingCount = 1,
				.pBindings = &descriptorSetLayoutBinding
		};
		descriptorSetLayout = core->device().createDescriptorSetLayout(descriptorSetLayoutCreateInfo, nullptr);

		vk::ShaderModule vertShader = createShader(resourceManager, "shaders/default.vert.spv");
		vk::ShaderModule fragShader = createShader(resourceManager, "shaders/default.frag.spv");

		vk::PushConstantRange constant {
			vk::ShaderStageFlagBits::eVertex, 0, sizeof(CameraTransform)
		};

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
				.setLayoutCount = 1,
				.pSetLayouts = &descriptorSetLayout,
				.pushConstantRangeCount = 1,
				.pPushConstantRanges = &constant
		};
		pipelineLayout = core->device().createPipelineLayout(pipelineLayoutCreateInfo, nullptr);

		vk::PipelineShaderStageCreateInfo stages[] {
				{.stage = vk::ShaderStageFlagBits::eVertex, .module = vertShader, .pName = "main"},
				{.stage = vk::ShaderStageFlagBits::eFragment, .module = fragShader, .pName = "main"},
		};

		vk::VertexInputBindingDescription bindings[] {
				{0, sizeof(Vertex), vk::VertexInputRate::eVertex}
		};

		vk::VertexInputAttributeDescription attributes[]{
				{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
				{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
				{2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, coords)},
		};

		vk::PipelineVertexInputStateCreateInfo vertexInputState{
				.vertexBindingDescriptionCount = std::size(bindings),
				.pVertexBindingDescriptions = bindings,
				.vertexAttributeDescriptionCount = std::size(attributes),
				.pVertexAttributeDescriptions = attributes
		};

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{
				.topology = vk::PrimitiveTopology::eTriangleList,
				.primitiveRestartEnable = false
		};

		vk::PipelineViewportStateCreateInfo viewportState{
				.viewportCount = 1,
				.pViewports = &viewport,
				.scissorCount = 1,
				.pScissors = &scissor
		};

		vk::PipelineRasterizationStateCreateInfo rasterizationState{
				.polygonMode = vk::PolygonMode::eFill,
				.cullMode = vk::CullModeFlagBits::eNone,
				.frontFace = vk::FrontFace::eCounterClockwise,
				.lineWidth = 1.0f
		};

		vk::PipelineMultisampleStateCreateInfo multisampleState {};

		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{
				.blendEnable = VK_TRUE,
				.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
				.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
				.colorBlendOp = vk::BlendOp::eAdd,
				.srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
				.dstAlphaBlendFactor = vk::BlendFactor::eZero,
				.alphaBlendOp = vk::BlendOp::eAdd,
				.colorWriteMask = vk::ColorComponentFlagBits::eR |
						vk::ColorComponentFlagBits::eG |
						vk::ColorComponentFlagBits::eB |
						vk::ColorComponentFlagBits::eA
		};

		vk::PipelineDepthStencilStateCreateInfo depthStencilState {
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = vk::CompareOp::eLess
		};

		vk::PipelineColorBlendStateCreateInfo colorBlendState{
				.attachmentCount = 1,
				.pAttachments = &colorBlendAttachmentState
		};

		vk::DynamicState dynamicStates[] {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamicState {
			.dynamicStateCount = 2,
			.pDynamicStates = dynamicStates
		};

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
				.stageCount = 2,
				.pStages = stages,
				.pVertexInputState = &vertexInputState,
				.pInputAssemblyState = &inputAssemblyState,
				.pViewportState = &viewportState,
				.pRasterizationState = &rasterizationState,
				.pMultisampleState = &multisampleState,
				.pDepthStencilState = &depthStencilState,
				.pColorBlendState = &colorBlendState,
				.pDynamicState = &dynamicState,
				.layout = pipelineLayout,
				.renderPass = renderer->renderPass,
		};

		core->device().createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
		core->device().destroyShaderModule(vertShader, nullptr);
		core->device().destroyShaderModule(fragShader, nullptr);

		resourceManager->loadFile("resource_packs/vanilla/textures/entity/agent.png", [&](std::span<const char> bytes) {
			auto cmd = renderer->commandPool.allocate(vk::CommandBufferLevel::ePrimary);
			cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

			int width, height, channels;

			auto data = reinterpret_cast<const unsigned char*>(bytes.data());
			auto pixels = stbi_load_from_memory(data, bytes.size(), &width, &height, &channels, 0);

			texture = Texture::create2D(vk::Format::eR8G8B8A8Unorm, width, height);
			texture.updateDescriptorSet(renderer->descriptorPool.allocate(descriptorSetLayout));

			vk::DeviceSize upload_size = width * height * channels;

			{
				vk::BufferCreateInfo srcBufferCreateInfo{.size = upload_size, .usage = vk::BufferUsageFlagBits::eTransferSrc};

				auto srcBuffer = Buffer::create(srcBufferCreateInfo, {.usage = VMA_MEMORY_USAGE_CPU_ONLY});
				std::memcpy(srcBuffer.map(), pixels, upload_size);
				srcBuffer.unmap();

				vk::ImageMemoryBarrier copy_barrier{
						.dstAccessMask = vk::AccessFlagBits::eTransferWrite,
						.oldLayout = vk::ImageLayout::eUndefined,
						.newLayout = vk::ImageLayout::eTransferDstOptimal,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = texture.image,
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

				cmd.copyBufferToImage(srcBuffer, texture.image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

				vk::ImageMemoryBarrier use_barrier{
						.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
						.dstAccessMask = vk::AccessFlagBits::eShaderRead,
						.oldLayout = vk::ImageLayout::eTransferDstOptimal,
						.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = texture.image,
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

				srcBuffer.destroy();
			}
		});

		resourceManager->loadFile("resource_packs/vanilla/models/mobs.json", [&](std::span<const char> bytes) {
			json::Parser parser(bytes);

			auto mobs = parser.parse().value().object();

			auto obj = mobs.at("geometry.agent");
			{
				auto&& mob_cfg = obj.object();

				int texture_width = mob_cfg.at("texturewidth").i64();
				int texture_height = mob_cfg.at("textureheight").i64();

				auto bones_cfg = mob_cfg.at("bones").array_view();
				for (auto&& bone_obj : bones_cfg) {
					auto&& bone_cfg = bone_obj.object();

					auto name = std::move(bone_cfg.at("name").string());

					bool neverRender = false;
					if (bone_cfg.contains("neverRender")) {
						neverRender = bone_cfg.at("neverRender").as_bool();
					}

					if (bone_cfg.contains("cubes")) {
						auto cubes_cfg = bone_cfg.at("cubes").array_view();

						for (auto &&cube_obj : cubes_cfg) {
							auto &&cube_cfg = cube_obj.object();
							auto origin = cube_cfg.at("origin").array_view();
							auto size = cube_cfg.at("size").array_view();
							auto uv = cube_cfg.at("uv").array_view();

							if (!neverRender) {
								const float ox = origin[0].f64() / 16.0f;
								const float oy = origin[1].f64() / 16.0f;
								const float oz = origin[2].f64() / 16.0f;

								const float sx = size[0].f64();
								const float sy = size[1].f64();
								const float sz = size[2].f64();

								const float x0 = ox;
								const float y0 = oy;
								const float z0 = oz;

								const float x1 = ox + sx / 16.0f;
								const float y1 = oy + sy / 16.0f;
								const float z1 = oz + sz / 16.0f;

								const float u = uv[0].f64();
								const float v = uv[1].f64();

								const float u0 = u;
								const float u1 = u + sz;
								const float u2 = u + sz + sx;
								const float u3 = u + sz + sx + sx;
								const float u4 = u + sz + sx + sz;
								const float u5 = u + sz + sx + sz + sx;

								const float v0 = v;
								const float v1 = v + sz;
								const float v2 = v + sz + sy;

								PositionTextureVertex vertex0{x0, y0, z0};
								PositionTextureVertex vertex1{x0, y1, z0};
								PositionTextureVertex vertex2{x1, y1, z0};
								PositionTextureVertex vertex3{x1, y0, z0};

								PositionTextureVertex vertex4{x1, y1, z1};
								PositionTextureVertex vertex5{x1, y0, z1};
								PositionTextureVertex vertex6{x0, y1, z1};
								PositionTextureVertex vertex7{x0, y0, z1};

								TexturedQuad quad0({ vertex0, vertex1, vertex2, vertex3 }, u1, v1, u2, v2, texture_width, texture_height, {0, 0, -1});
								TexturedQuad quad1({ vertex3, vertex2, vertex4, vertex5 }, u1, v1, u0, v2, texture_width, texture_height, {1, 0, 0});
								TexturedQuad quad2({ vertex5, vertex4, vertex6, vertex7 }, u4, v1, u5, v2, texture_width, texture_height, {0, 0, 1});
								TexturedQuad quad3({ vertex7, vertex6, vertex1, vertex0 }, u0, v1, u1, v2, texture_width, texture_height, {-1, 0, 0});
								TexturedQuad quad4({ vertex1, vertex6, vertex4, vertex2 }, u1, v0, u2, v1, texture_width, texture_height, {0, 1, 0});
								TexturedQuad quad5({ vertex7, vertex0, vertex3, vertex5 }, u2, v0, u3, v1, texture_width, texture_height, {0, -1, 0});

								ModelBox modelBox {
									.quads { quad0, quad1, quad2, quad3, quad4, quad5 }
								};

								for (auto&& quad : modelBox.quads) {
									auto normal = quad._normal;

									vertexBuilder.addQuad(0, 1, 2, 0, 2, 3);
									for (auto&& vertex : quad._vertices) {
										vertexBuilder.vertices.emplace_back(
											vertex.x, vertex.y, vertex.z,
											vertex.u, vertex.v,
											normal.x, normal.y, normal.z
										);
									}
								}
							}
						}
					}
				}
			}

			renderBuffer.SetIndexBufferSize(sizeof(int) * vertexBuilder.indices.size());
			renderBuffer.SetVertexBufferSize(sizeof(Vertex) * vertexBuilder.vertices.size());

			renderBuffer.SetIndexBufferData(vertexBuilder.indices.data(), 0, 0, sizeof(int) * vertexBuilder.indices.size());
			renderBuffer.SetVertexBufferData(vertexBuilder.vertices.data(), 0, 0, sizeof(Vertex) * vertexBuilder.vertices.size());
		});
	}

	~AgentEntity() {
		texture.destroy();

		core->device().destroyPipelineLayout(pipelineLayout, nullptr);
		core->device().destroyPipeline(pipeline, nullptr);
		renderBuffer.destroy();
	}

	inline vk::ShaderModule createShader(ResourceManager* rm, const std::string& path) {
		vk::ShaderModule shader;
		rm->loadFile(path, [&](std::span<char> bytes) {
			shader = core->device().createShaderModule({
					.codeSize = bytes.size(),
					.pCode = reinterpret_cast<const uint32_t *>(bytes.data())
			});
		});
		return shader;
	}

	void setRenderingSize(int width, int height) {
		viewport.width = width;
		viewport.height = height;

		scissor.extent.width = width;
		scissor.extent.height = height;
	}

	void render(vk::CommandBuffer cmd, CameraTransform& transform) {
		vk::DeviceSize offset{0};

		vk::Buffer vertexBuffers[] {
			renderBuffer.VertexBuffer
		};

		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		cmd.setViewport(0, 1, &viewport);
		cmd.setScissor(0, 1, &scissor);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &texture.descriptor, 0, nullptr);
		cmd.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(CameraTransform), &transform);
		cmd.bindVertexBuffers(0, 1, vertexBuffers, &offset);
		cmd.bindIndexBuffer(renderBuffer.IndexBuffer, 0, vk::IndexType::eUint32);

		cmd.drawIndexed(vertexBuilder.indices.size(), 1, 0, 0, 0);
	}

private:
	RenderSystem* core = RenderSystem::Get();

	vk::PipelineLayout pipelineLayout;
	vk::Pipeline pipeline;
	vk::Sampler sampler;

	Texture texture;
	vk::DescriptorSetLayout descriptorSetLayout;

	RenderBuffer renderBuffer;
	VertexBuilder vertexBuilder;

	vk::Viewport viewport{
		.x = 0,
		.y = 0,
		.width = 0,
		.height = 0,
		.minDepth = 0,
		.maxDepth = 1
	};

	vk::Rect2D scissor {};
};

struct GameClient {
	void init(GameWindow* window) {
		resourceManager = std::make_unique<ResourceManager>("assets");
		renderer = std::make_unique<GameRenderer>(window);
		agent = std::make_unique<AgentEntity>(resourceManager.get(), renderer.get());
	}

	void tick() {
		clock.update();

		Mouse::update();
		Keyboard::update();

		glfwPollEvents();

		camera_tick(clock.deltaSeconds());

		auto proj = projection;
		auto view = rotationMatrix();

		CameraTransform transform {
			.camera = proj * glm::translate(view, -cameraPosition)
		};

		auto cmd = renderer->begin();
		agent->render(cmd, transform);
		renderer->end();
	}

	void setRenderingSize(int width, int height) {
		projection = PerspectiveProjectionMatrix(
				glm::radians(60.0f),
				float(width) / float(height),
				0.1f,
				1000.0f
		);

		agent->setRenderingSize(width, height);
		renderer->setRenderingSize(width, height);
	}

private:
	glm::mat4 rotationMatrix() {
		float yaw = glm::radians(rotationYaw);
		float pitch = glm::radians(rotationPitch);

		float sp = glm::sin(pitch);
		float cp = glm::cos(pitch);

		float c = glm::cos(yaw);
		float s = glm::sin(yaw);

		glm::mat4 view;
		view[0] = glm::vec4(c, sp * s, -cp * s, 0);
		view[1] = glm::vec4(0, cp, sp, 0);
		view[2] = glm::vec4(s, -sp * c, cp * c, 0);
		view[3] = glm::vec4(0, 0, 0, 1);
		return view;
	}

	void camera_tick(float dt) {
		if (Mouse::IsButtonPressed(MouseButton::Left)) {
//			_input->setCursorState(CursorState::Locked);

			float xdelta, ydelta;
			Mouse::getDelta(xdelta, ydelta);

			if (xdelta != 0 || ydelta != 0) {
				double d4 = 0.5f * (double) 0.6F + (double) 0.2F;
				double d5 = d4 * d4 * d4 * 8.0;

				rotationYaw = rotationYaw - xdelta * d5 * dt * 9.0;
				rotationPitch = glm::clamp(rotationPitch - ydelta * d5 * dt * 9.0, -90.0, 90.0);
			}
		} else {
//			_input->setCursorState(CursorState::Normal);
		}

		auto rot = glm::mat3(rotationMatrix());
		auto forward = glm::vec3(0, 0, 1) * rot;
		auto right = glm::vec3(1, 0, 0) * rot;

		if (Keyboard::IsKeyPressed(Key::W)) {
			cameraPosition += forward * dt * 5.0f;
		}
		if (Keyboard::IsKeyPressed(Key::S)) {
			cameraPosition -= forward * dt * 5.0f;
		}
		if (Keyboard::IsKeyPressed(Key::A)) {
			cameraPosition -= right * dt * 5.0f;
		}
		if (Keyboard::IsKeyPressed(Key::D)) {
			cameraPosition += right * dt * 5.0f;
		}
	}
private:
	Clock clock;

	std::unique_ptr<ResourceManager> resourceManager;
	std::unique_ptr<GameRenderer> renderer;
	std::unique_ptr<AgentEntity> agent;

	glm::mat4 projection;
	glm::vec3 cameraPosition{0, 1.5f, -2};
	float rotationYaw{0};
	float rotationPitch{0};
};

int main(int, char**) {
	GameWindow window("Vulkan", 1280, 720);
	window.setMouseButtonCallback(&Mouse::handleMouseButton);
	window.setMousePositionCallback(&Mouse::handleMousePosition);
	window.setKeyboardCallback(&Keyboard::handleKeyInput);

	int width, height;
	window.getWindowSize(width, height);

	GameClient client{};
	client.init(&window);
	client.setRenderingSize(width, height);

	while (!window.wantToQuit()) {
		client.tick();
	}
	return 0;
}