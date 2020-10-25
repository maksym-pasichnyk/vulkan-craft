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

struct TextureUVCoordinateSet {
    float minU;
    float minV;
    float maxU;
    float maxV;

    inline constexpr float getInterpolatedU(float t) const {
        return (1 - t) * minU + t * maxU;
    }

    inline constexpr float getInterpolatedV(float t) const {
        return (1 - t) * minV + t * maxV;
    }
};

struct AABB {
	float minX;
	float minY;
	float minZ;
	float maxX;
	float maxY;
	float maxZ;

	void set(float x1, float y1, float z1, float x2, float y2, float z2) {
		minX = x1;
		minY = y1;
		minZ = z1;
		maxX = x2;
		maxY = y2;
		maxZ = z2;
	}
};

struct Color32 {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

struct Tile {
	virtual ~Tile() = default;

	virtual void getVisualShape(char data, AABB& aabb) {
		aabb = visualShape;
	}

	virtual void getVisualShape(int data, int x, int y, int z, AABB& aabb) {
		aabb = visualShape;
	}

	void setVisualShape(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z) {
		visualShape.set(min_x, min_y, min_z, max_x, max_y, max_z);
	}

	virtual TextureUVCoordinateSet getTexture(unsigned char side, int data) {
		return {0, 0, 1, 1};
	}

private:
	AABB visualShape;
};

struct ButtonTile : Tile {
	void getVisualShape(int data, int x, int y, int z, AABB& shape) override {
		int rot = data & 7;
		bool powered = (data & 8) > 0;

		float f = 0.375F;
		float f1 = 0.625F;
		float f2 = 0.1875F;
		float f3 = 0.125F;

		if (powered) f3 = 0.0625F;

		if (rot == 1)
			shape.set(0.0F, f, 0.5F - f2, f3, f1, 0.5F + f2);
		else if (rot == 2)
			shape.set(1.0F - f3, f, 0.5F - f2, 1.0F, f1, 0.5F + f2);
		else if (rot == 3)
			shape.set(0.5F - f2, f, 0.0F, 0.5F + f2, f1, f3);
		else if (rot == 4)
			shape.set(0.5F - f2, f, 1.0F - f3, 0.5F + f2, f1, 1.0F);
		else if (rot == 5)
			shape.set(f, 0.0F, 0.5F - f2, f1, f3, 0.5F + f2);
		else if (rot == 6)
			shape.set(f, 1.0F - f3, 0.5F - f2, f1, 1.0F, 0.5F + f2);
	}
};

struct PistonBaseTile : Tile {
	void getVisualShape(int data, int x, int y, int z, AABB& shape) override {
		if (isPowered(data)) {
			switch (getRotation(data)) {
			case 0:
				shape.set(0.0F, 0.25F, 0.0F, 1.0F, 1.0F, 1.0F);
				break;
			case 1:
				shape.set(0.0F, 0.0F, 0.0F, 1.0F, 0.75F, 1.0F);
				break;
			case 2:
				shape.set(0.0F, 0.0F, 0.25F, 1.0F, 1.0F, 1.0F);
				break;
			case 3:
				shape.set(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 0.75F);
				break;
			case 4:
				shape.set(0.25F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
				break;
			case 5:
				shape.set(0.0F, 0.0F, 0.0F, 0.75F, 1.0F, 1.0F);
				break;
			}
		} else
			shape.set(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
	}

	inline static int getRotation(int data) {
		return data & 7;
	}

	inline static bool isPowered(int data) {
		return (data & 8) != 0;
	}
};

struct PistonArmTile : Tile {
	void getVisualShape(int data, int x, int y, int z, AABB& shape) override {
		switch(getRotation(data)) {
		case 0:
			shape.set(0.0F, 0.0F, 0.0F, 1.0F, 0.25F, 1.0F);
			break;
		case 1:
			shape.set(0.0F, 0.75F, 0.0F, 1.0F, 1.0F, 1.0F);
			break;
		case 2:
			shape.set(0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 0.25F);
			break;
		case 3:
			shape.set(0.0F, 0.0F, 0.75F, 1.0F, 1.0F, 1.0F);
			break;
		case 4:
			shape.set(0.0F, 0.0F, 0.0F, 0.25F, 1.0F, 1.0F);
			break;
		case 5:
			shape.set(0.75F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F);
			break;
		}
	}

	inline static int getRotation(int data) {
		return data & 7;
	}
};

struct Tiles {
	inline static auto button = new ButtonTile();
	inline static auto pistonBase = new PistonBaseTile();
	inline static auto pistonArm = new PistonArmTile();
};

enum class ConnectionBit {
    None = 0,
    West = 1,
    North = 2,
    East = 4,
    South = 8,
};

inline constexpr auto operator&&(ConnectionBit __lhs, ConnectionBit __rhs) -> bool {
    using base = std::underlying_type<ConnectionBit>::type;
    return (base(__lhs) & base(__rhs)) != base(0);
}

inline constexpr auto operator|(ConnectionBit __lhs, ConnectionBit __rhs) -> ConnectionBit {
    using base = std::underlying_type<ConnectionBit>::type;
    return ConnectionBit(base(__lhs) | base(__rhs));
}

enum class FlammableBit {
    Up = 1,
    Down = 2,
    North = 4,
    South = 8,
    East = 16,
    West = 32
};

inline constexpr auto operator&&(FlammableBit __lhs, FlammableBit __rhs) -> bool {
    using base = std::underlying_type<FlammableBit>::type;
    return (base(__lhs) & base(__rhs)) != base(0);
}

inline constexpr auto operator|(FlammableBit __lhs, FlammableBit __rhs) -> FlammableBit {
    using base = std::underlying_type<FlammableBit>::type;
    return FlammableBit(base(__lhs) | base(__rhs));
}

inline constexpr auto operator|=(FlammableBit& __lhs, FlammableBit __rhs) -> FlammableBit& {
    using base = std::underlying_type<FlammableBit>::type;
    return __lhs = FlammableBit(base(__lhs) | base(__rhs));
}

// todo: rotate texture
struct Tessellator {
	std::vector<int> indices;
	std::vector<Vertex> vertices;
    glm::vec3 _normal;
    glm::vec3 _offset;
    Color32 _color;

    inline static Tessellator* instance() {
    	static Tessellator self;
    	return &self;
    }

    inline void normal(glm::vec3 normal) {
        _normal = normal;
    }

	inline void normal(float x, float y, float z) {
        _normal.x = x;
        _normal.y = y;
        _normal.z = z;
    }

	inline void addOffset(float x, float y, float z) {
        _offset.x += x;
        _offset.y += y;
        _offset.z += z;
    }

	inline void setOffset(float x, float y, float z) {
        _offset.x = x;
        _offset.y = y;
        _offset.z = z;
    }

    inline void color(Color32 color) {
        _color = color;
    }

    inline void color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 0xFF) {
        _color.r = r;
        _color.g = g;
        _color.b = b;
        _color.a = a;
    }

    inline void quad() {
		quad(0, 1, 2, 0, 2, 3);
    }

    inline void quadInv() {
		quad(0, 2, 1, 0, 3, 2);
    }

	inline void quad(int a, int b, int c) {
		int idx = vertices.size();

		indices.reserve(3);

		indices.push_back(idx + a);
		indices.push_back(idx + b);
		indices.push_back(idx + c);
    }

	void quad(int a1, int b1, int c1, int a2, int b2, int c2) {
		int idx = vertices.size();

		indices.reserve(6);

		indices.push_back(idx + a1);
		indices.push_back(idx + b1);
		indices.push_back(idx + c1);

		indices.push_back(idx + a2);
		indices.push_back(idx + b2);
		indices.push_back(idx + c2);
	}

	inline void vertexUV(float x, float y, float z, float u, float v) {
		vertices.emplace_back(x + _offset.x, y + _offset.y, z + _offset.z, u, v, _normal.x, _normal.y, _normal.z);
    }

	inline void vertexUV(float x, float y, float z, float u, float v, unsigned char light) {
		vertices.emplace_back(x + _offset.x - 0.5f, y + _offset.y - 0.5f, z + _offset.z - 0.5f, u, v, _normal.x, _normal.y, _normal.z);
    }
};

struct ModelTransform {
	ModelTransform* next{nullptr};

	float origin[3]{ 0.5f, 0.5f, 0.5f };
	float rotX = 0;
	float rotY = 0;
	float rotZ = 0;

	inline void apply(float p1[4], float p2[4], float p3[4], float p4[4]) {
		if (rotX != 0) rotateX(rotX, origin, p1, p2, p3, p4);
		if (rotY != 0) rotateY(rotY, origin, p1, p2, p3, p4);
		if (rotZ != 0) rotateZ(rotZ, origin, p1, p2, p3, p4);
	}

	inline static void rotate(int ix, int iy, float angle, const float (&origin)[3], float *p1, float *p2, float *p3, float *p4) {
        float ox = origin[ix];
        float oy = origin[iy];

        float s = std::sin(angle);
        float c = std::cos(angle);

		float x1 = p1[ix] - ox;
		float y1 = p1[iy] - oy;
		p1[ix] = ox + x1 * c + y1 * s;
		p1[iy] = oy - x1 * s + y1 * c;

		float x2 = p2[ix] - ox;
		float y2 = p2[iy] - oy;
		p2[ix] = ox + x2 * c + y2 * s;
		p2[iy] = oy - x2 * s + y2 * c;

		float x3 = p3[ix] - ox;
		float y3 = p3[iy] - oy;
		p3[ix] = ox + x3 * c + y3 * s;
		p3[iy] = oy - x3 * s + y3 * c;

		float x4 = p4[ix] - ox;
		float y4 = p4[iy] - oy;
		p4[ix] = ox + x4 * c + y4 * s;
		p4[iy] = oy - x4 * s + y4 * c;
    }

    inline static void rotateX(float angle, const float(&origin)[3], float p1[4], float p2[4], float p3[4], float p4[4]) {
		rotate(2, 1, angle, origin, p1, p2, p3, p4);
    }

    inline static void rotateY(float angle, const float (&origin)[3], float p1[4], float p2[4], float p3[4], float p4[4]) {
		rotate(0, 2, angle, origin, p1, p2, p3, p4);
    }

    inline static void rotateZ(float angle, const float(&origin)[3], float p1[4], float p2[4], float p3[4], float p4[4]) {
		rotate(1, 0, angle, origin, p1, p2, p3, p4);
    }
};

struct BlockTessellator {
	Tessellator* tessellator = Tessellator::instance();
	AABB bounds;

	ModelTransform* transform;

    bool useForcedUV = false;
	TextureUVCoordinateSet forcedUV;

	void setRenderBounds(int rotation, float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
		bounds.minX = minX;
		bounds.minY = minY;
		bounds.minZ = minZ;
		bounds.maxX = maxX;
		bounds.maxY = maxY;
		bounds.maxZ = maxZ;
	}

	void setRotatedBounds(int rotation, float x1, float y1, float z1, float x2, float y2, float z2) {
		bounds.set(x1, y1, z1, x2, y2, z2);

        switch (rotation) {
        case 2: // south
        	bounds.minX = x1;
        	bounds.minY = y1;
        	bounds.minZ = z1;
        	bounds.maxX = x2;
        	bounds.maxY = y2;
        	bounds.maxZ = z2;
			break;
        case 3: // north
        	bounds.minX = 1 - x2;
        	bounds.minY = y1;
        	bounds.minZ = 1 - z2;
        	bounds.maxX = 1 - x1;
        	bounds.maxY = y2;
        	bounds.maxZ = 1 - z1;
			break;
        case 4: // east
        	bounds.minX = 1 - z2;
        	bounds.minY = y1;
        	bounds.minZ = x1;
        	bounds.maxX = 1 - z1;
        	bounds.maxY = y2;
        	bounds.maxZ = x2;
			break;
        case 5: // west
        	bounds.minX = z1;
        	bounds.minY = y1;
        	bounds.minZ = 1 - x2;
        	bounds.maxX = z2;
        	bounds.maxY = y2;
        	bounds.maxZ = 1 - x1;
			break;
//        case 4: // up
//        	bounds.minX = x1;
//        	bounds.minY = 1 - z2;
//        	bounds.minZ = y1;
//        	bounds.maxX = x2;
//        	bounds.maxY = 1 - z1;
//        	bounds.maxZ = y2;
//			break;
//        case 5: // down
//        	bounds.minX = x1;
//        	bounds.minY = z1;
//        	bounds.minZ = 1 - y2;
//        	bounds.maxX = x2;
//        	bounds.maxY = z2;
//        	bounds.maxZ = 1 - y1;
//			break;
		}
	}

	void setRotatedBounds(int rotation, const AABB& aabb) {
		bounds = aabb;

		switch (rotation) {
        case 3:
        	bounds.minX = 1 - aabb.maxX;
        	bounds.minY = aabb.minY;
        	bounds.minZ = 1 - aabb.maxZ;
        	bounds.maxX = 1 - aabb.minX;
        	bounds.maxY = aabb.maxY;
        	bounds.maxZ = 1 - aabb.minZ;
			break;
        case 2:
        	bounds.minX = aabb.minX;
        	bounds.minY = aabb.minY;
        	bounds.minZ = aabb.minZ;
        	bounds.maxX = aabb.maxX;
        	bounds.maxY = aabb.maxY;
        	bounds.maxZ = aabb.maxZ;
			break;
        case 4:
        	bounds.minX = aabb.minZ;
        	bounds.minY = aabb.minY;
        	bounds.minZ = aabb.minX;
        	bounds.maxX = aabb.maxZ;
        	bounds.maxY = aabb.maxY;
        	bounds.maxZ = aabb.maxX;
			break;
        case 5:
        	bounds.minX = 1 - aabb.maxZ;
        	bounds.minY = aabb.minY;
        	bounds.minZ = 1 - aabb.maxX;
        	bounds.maxX = 1 - aabb.minZ;
        	bounds.maxY = aabb.maxY;
        	bounds.maxZ = 1 - aabb.minX;
			break;
//        case 4:
//        	bounds.minX = aabb.minX;
//        	bounds.minY = 1 - aabb.maxZ;
//        	bounds.minZ = aabb.minY;
//        	bounds.maxX = aabb.maxX;
//        	bounds.maxY = 1 - aabb.minZ;
//        	bounds.maxZ = aabb.maxY;
//			break;
//        case 5:
//        	bounds.minX = aabb.minX;
//        	bounds.minY = aabb.minZ;
//        	bounds.minZ = 1 - aabb.maxY;
//        	bounds.maxX = aabb.maxX;
//        	bounds.maxY = aabb.maxZ;
//        	bounds.maxZ = 1 - aabb.minY;
//			break;
		}
	}

	void setRenderBox(float x1, float y1, float z1, float x2, float y2, float z2) {
		bounds.set(x1 / 16.0f, y1 / 16.0f, z1 / 16.0f, x2 / 16.0f, y2 / 16.0f, z2 / 16.0f);
	}

	void setRotatedBox(int rotation, float x1, float y1, float z1, float x2, float y2, float z2) {
		setRotatedBounds(rotation, x1 / 16.0f, y1 / 16.0f, z1 / 16.0f, x2 / 16.0f, y2 / 16.0f, z2 / 16.0f);
	}

    inline void tessellateBlockInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);

        float p1[] { bounds.minX, bounds.minY, bounds.minZ };
	 	float p2[] { bounds.minX, bounds.minY, bounds.maxZ };
        float p3[] { bounds.maxX, bounds.minY, bounds.maxZ };
        float p4[] { bounds.maxX, bounds.minY, bounds.minZ };
		float p5[] { bounds.minX, bounds.maxY, bounds.minZ };
        float p6[] { bounds.minX, bounds.maxY, bounds.maxZ };
        float p7[] { bounds.maxX, bounds.maxY, bounds.maxZ };
        float p8[] { bounds.maxX, bounds.maxY, bounds.minZ };

        apply(transform, p1, p2, p3, p4);
        apply(transform, p5, p6, p7, p8);

		tessellator->normal(0, 1, 0);
		tessellator->quad();
		face(x, y, z, p5, p6, p7, p8, {0, 0, 1, 1});

		tessellator->normal(0, -1, 0);
		tessellator->quad();
		face(x, y, z, p2, p1, p4, p3, {0, 0, 1, 1});

		tessellator->normal(0, 0, 1);
		tessellator->quad();
		face(x, y, z, p3, p7, p6, p2, {0, 0, 1, 1});

		tessellator->normal(0, 0, -1);
		tessellator->quad();
		face(x, y, z, p1, p5, p8, p4, {0, 0, 1, 1});

		tessellator->normal(1, 0, 0);
		tessellator->quad();
		face(x, y, z, p4, p8, p7, p3, {0, 0, 1, 1});

		tessellator->normal(-1, 0, 0);
		tessellator->quad();
		face(x, y, z, p2, p6, p5, p1, {0, 0, 1, 1});
    }

    inline void tessellateAnvilInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//    	tessellator->setMaterial(material);

		int rotation = 2;

		setRotatedBox(rotation, 0, 0, 4, 16, 1, 12);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 1, 0, 3, 15, 1, 4);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 1, 0, 12, 15, 1, 13);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 1, 1, 4, 15, 4, 12);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 4, 4, 5, 12, 5, 12);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 6, 5, 5, 10, 10, 12);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 2, 10, 4, 14, 16, 12);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 14, 11, 4, 16, 15, 12);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 0, 11, 4, 2, 15, 12);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 3, 11, 3, 13, 15, 4);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 3, 11, 12, 13, 15, 13);
		tessellateBlockInWorld(tile, x, y, z);
    }

	inline void tessellateSpawnerInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);

		setRenderBox(1, 1, 1, 15, 15, 15);
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void bed() {}

	inline void tessellateBrewingStand(Tile* tile, int x, int y, int z) {
//		ModelTransform RotY {
//			.rotY = glm::radians(-180.0f)
//		};

//		transform = &RotY;

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);

		int rotation = 2;

		setRotatedBox(rotation, 0, 0, 0, 16, 2, 16);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 6, 2, 0, 10, 15, 2);
		tessellateBlockInWorld(tile, x, y, z);

//		bounds.set(0.25, 0.9375, 0.125, 0.75, 0.9375, 0.625);
//		tessellateUp(tile, x, y, z);
//
//		bounds.set(0.125, 0.9375, 0.3125, 0.875, 0.9375, 0.4375);
//		tessellateUp(tile, x, y, z);
//
//		bounds.set(0.4375, 0.9375, 0.625, 0.5625, 0.9375, 0.75);
//		tessellateUp(tile, x, y, z);

//		bounds.set(3 / 16.0f, 2 / 16.0f, 4 / 16.0f, 13 / 16.0f, 14.8f / 16.0f, 4 / 16.0f);
//		tessellateNorth(tile, x, y, z);
//		tessellateSouth(tile, x, y, z);
	}

	inline void tessellateButtonInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);

		float f = 6;
		float f1 = 10;
		float f2 = 3;

//		bool powered = false;
		float f3 = /*powered ? 1 :*/ 2;

		int data = 0;

		switch (data) {
		case 0:
			setRenderBox(f, 16 - f3, 8 - f2, f1, 16, 8 + f2);
			break;
		case 1:
			setRenderBox(0, f, 8 - f2, f3, f1, 8 + f2);
			break;
		case 2:
			setRenderBox(16 - f3, f, 8 - f2, 16, f1, 8 + f2);
			break;
		case 3:
			setRenderBox(8 - f2, f, 16 - f3, 8 + f2, f1, 16);
			break;
		case 4:
			setRenderBox(8 - f2, f, 0, 8 + f2, f1, f3);
			break;
		case 5:
			setRenderBox(f, 0, 8 - f2, f1, f3, 8 + f2);
			break;
		}
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateFenceInWorld(Tile* tile, int x, int y, int z) {
		ConnectionBit connections = ConnectionBit::South | ConnectionBit::East | ConnectionBit::North | ConnectionBit::West;

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setTile(getTile(mb_cfg, texture));
//		tessellator->setMaterial(material);

		bounds.set(0.3f, 0, 0.3f, 0.7f, 1, 0.7f);
		tessellateBlockInWorld(tile, x, y, z);

		if (connections && ConnectionBit::South) {
			bounds.set(0.4f, 0.3f, 0, 0.6f, 0.5f, 0.3f);
			tessellateBlockInWorld(tile, x, y, z);

			bounds.set(0.4f, 0.7f, 0, 0.6f, 0.9f, 0.3f);
			tessellateBlockInWorld(tile, x, y, z);
		}

		if (connections && ConnectionBit::East) {
			bounds.set(0.7f, 0.3f, 0.4f, 1.0f, 0.5f, 0.6f);
			tessellateBlockInWorld(tile, x, y, z);

			bounds.set(0.7f, 0.7f, 0.4f, 1.0f, 0.9f, 0.6f);
			tessellateBlockInWorld(tile, x, y, z);
		}

		if (connections && ConnectionBit::North) {
			bounds.set(0.4f, 0.3f, 0.7f, 0.6f, 0.5f, 1.0f);
			tessellateBlockInWorld(tile, x, y, z);

			bounds.set(0.4f, 0.7f, 0.7f, 0.6f, 0.9f, 1.0f);
			tessellateBlockInWorld(tile, x, y, z);
		}

		if (connections && ConnectionBit::West) {
			bounds.set(0, 0.3f, 0.4f, 0.3f, 0.5f, 0.6f);
			tessellateBlockInWorld(tile, x, y, z);

			bounds.set(0, 0.7f, 0.4f, 0.3f, 0.9f, 0.6f);
			tessellateBlockInWorld(tile, x, y, z);
		}
	}

	inline void tessellateWallInWorld(Tile* tile, int x, int y, int z) {
		ConnectionBit connections = ConnectionBit::South | ConnectionBit::East | ConnectionBit::North | ConnectionBit::West;

    	bounds.set(0.2f, 0, 0.2f, 0.8f, 1, 0.8f);
		tessellateBlockInWorld(tile, x, y, z);

		if (connections && ConnectionBit::South) {
			bounds.set(0.3f, 0, 0, 0.7f, 0.8f, 0.2f);
			tessellateBlockInWorld(tile, x, y, z);
		}

		if (connections && ConnectionBit::East) {
			bounds.set(0.8f, 0, 0.3f, 1.0f, 0.8f, 0.7f);
			tessellateBlockInWorld(tile, x, y, z);
		}

		if (connections && ConnectionBit::North) {
			bounds.set(0.3f, 0, 0.8f, 0.7f, 0.8f, 1.0f);
			tessellateBlockInWorld(tile, x, y, z);
		}

		if (connections && ConnectionBit::West) {
			bounds.set(0, 0, 0.3f, 0.2f, 0.8f, 0.7f);
			tessellateBlockInWorld(tile, x, y, z);
		}
    }

	inline void tessellateFenceGateInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setTile(getTile(mb_cfg, texture));
//		tessellator->setMaterial(material);

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setTile(getTile(mb_cfg, texture));
//		tessellator->setMaterial(material);

		int rotation = 2;

		switch (/*m_res->cc_block->additional_data*/0) {
		case 0: {
			setRotatedBox(rotation, 0, 4, 7, 2, 15, 9);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 14, 4, 7, 16, 15, 9);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 2, 5, 7, 14, 8, 9);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 2, 11, 7, 14, 14, 9);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 6, 8, 7, 10, 11, 9);
			tessellateBlockInWorld(tile, x, y, z);

			break;
		}
		case 1: {
			setRotatedBox(rotation, 0, 4, 8, 2, 15, 10);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 0, 5, 14, 2, 14, 16);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 0, 5, 10, 2, 8, 14);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 0, 11, 10, 2, 14, 14);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 14, 4, 8, 16, 15, 10);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 14, 5, 14, 16, 14, 16);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 14, 5, 10, 16, 8, 14);
			tessellateBlockInWorld(tile, x, y, z);

			setRotatedBox(rotation, 14, 11, 10, 16, 14, 14);
			tessellateBlockInWorld(tile, x, y, z);

			break;
		}
		}
	}

	inline void tessellateTorchInWorld(Tile* tile, int x, int y, int z) {
	    tessellator->color(0xFF, 0xFF, 0xFF);

		ModelTransform Rot {};
		transform = &Rot;

		int data = 1;

		switch (data) {
		case 1:
			Rot.rotZ = glm::radians(-22.5f);
        	setRenderBox(-1, 3.5f, 7, 1.5f, 13.5f, 9.5f);
			break;
		case 2:
			Rot.rotZ = glm::radians(22.5f);
        	setRotatedBox(3, -1, 3.5f, 7, 1.5f, 13.5f, 9.5f);
			break;
		case 3:
			Rot.rotX = glm::radians(-22.5f);
        	setRotatedBox(5, -1, 3.5f, 7, 1.5f, 13.5f, 9.5f);
			break;
		case 4:
			Rot.rotX = glm::radians(22.5f);
			setRotatedBox(4, -1, 3.5f, 7, 1.5f, 13.5f, 9.5f);
			break;
		case 5:
			setRenderBox(6.75f, 0, 6.75f, 9.25f, 0.625, 9.25f);
			break;
		}
        tessellateBlockInWorld(tile, x, y, z);

        transform = nullptr;
	}

	inline void tessellatePaintingInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);

		int rotation = 2;
		setRotatedBounds(rotation, 0, 0, 0.9, 1, 1, 1);
        tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateLilyPadInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);
		bounds.set(0.07, 0.01, 0.07, 0.93, 0.01, 0.93);
		tessellateUp(tile, x, y, z);
	}

	inline void tessellateRailInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//    tessellator->setMaterial(material);

		int rotation = 2;
		int ascending[4]{0, 0, 0, 0};

////    TileCoordinateSet tile;
//    switch ((RailDirection)m_res->cc_block->additional_data) {
//    case RailDirection::EAST_WEST:
////        tile = getTile(mb_cfg, texture);
//        rotation = 1;
//        break;
//    case RailDirection::NORTH_SOUTH:
////        tile = getTile(mb_cfg, texture);
//        rotation = 0;
//        break;
//    case RailDirection::SOUTH_EAST:
////        tile = getTile(mb_cfg, texture, Direction::East);
//        rotation = 0;
//        break;
//    case RailDirection::SOUTH_WEST:
////        tile = getTile(mb_cfg, texture, Direction::East);
//        rotation = 3;
//        break;
//    case RailDirection::NORTH_WEST:
////        tile = getTile(mb_cfg, texture, Direction::East);
//        rotation = 2;
//        break;
//    case RailDirection::NORTH_EAST:
////        tile = getTile(mb_cfg, texture, Direction::East);
//        rotation = 1;
//        break;
//    case RailDirection::ASCENDING_EAST:
////        tile = getTile(mb_cfg, texture);
//        ascending[2] = 1;
//        ascending[3] = 1;
//        rotation = 1;
//        break;
//    case RailDirection::ASCENDING_WEST:
////        tile = getTile(mb_cfg, texture);
//        ascending[0] = 1;
//        ascending[1] = 1;
//        rotation = 1;
//        break;
//    case RailDirection::ASCENDING_SOUTH:
////        tile = getTile(mb_cfg, texture);
//        ascending[0] = 1;
//        ascending[3] = 1;
//        break;
//    case RailDirection::ASCENDING_NORTH:
////        tile = getTile(mb_cfg, texture);
//        ascending[1] = 1;
//        ascending[2] = 1;
//        rotation = 0;
//        break;
//
//    default:
//        tile = getTile(mb_cfg, texture);
//        break;
//    }

		TextureUVCoordinateSet rails = {0, 0, 1, 1};

		const float coords[]{
				rails.getInterpolatedU(0), rails.getInterpolatedV(0),
				rails.getInterpolatedU(0), rails.getInterpolatedV(1),
				rails.getInterpolatedU(1), rails.getInterpolatedV(1),
				rails.getInterpolatedU(1), rails.getInterpolatedV(0)
		};

		auto i0 = ((rotation + 0) & 3) << 1;
		auto i1 = ((rotation + 1) & 3) << 1;
		auto i2 = ((rotation + 2) & 3) << 1;
		auto i3 = ((rotation + 3) & 3) << 1;

		tessellator->quad();
//    tessellator->quadInv();
		tessellator->vertexUV(0 + x, ascending[0] + y, 0 + z, coords[i0], coords[i0 | 1], 15);
		tessellator->vertexUV(0 + x, ascending[1] + y, 1 + z, coords[i1], coords[i1 | 1], 15);
		tessellator->vertexUV(1 + x, ascending[2] + y, 1 + z, coords[i2], coords[i2 | 1], 15);
		tessellator->vertexUV(1 + x, ascending[3] + y, 0 + z, coords[i3], coords[i3 | 1], 15);
	}

	inline void tessellateFlowerInWorld(Tile* tile, int x, int y, int z) {
		TextureUVCoordinateSet flower = {0, 0, 1, 1};

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);

		auto min_u = flower.getInterpolatedU(0);
		auto min_v = flower.getInterpolatedV(0);
		auto max_u = flower.getInterpolatedU(1);
		auto max_v = flower.getInterpolatedV(1);

		tessellator->quad();
		tessellator->vertexUV(0 + x, 0 + y, 0 + z, min_u, min_v, 15);
		tessellator->vertexUV(0 + x, 1 + y, 0 + z, min_u, max_v, 15);
		tessellator->vertexUV(1 + x, 1 + y, 1 + z, max_u, max_v, 15);
		tessellator->vertexUV(1 + x, 0 + y, 1 + z, max_u, min_v, 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(0 + x, 0 + y, 0 + z, min_u, min_v, 15);
//		tessellator->vertexUV(0 + x, 1 + y, 0 + z, min_u, max_v, 15);
//		tessellator->vertexUV(1 + x, 1 + y, 1 + z, max_u, max_v, 15);
//		tessellator->vertexUV(1 + x, 0 + y, 1 + z, max_u, min_v, 15);

		tessellator->quad();
		tessellator->vertexUV(0 + x, 0 + y, 1 + z, min_u, min_v, 15);
		tessellator->vertexUV(0 + x, 1 + y, 1 + z, min_u, max_v, 15);
		tessellator->vertexUV(1 + x, 1 + y, 0 + z, max_u, max_v, 15);
		tessellator->vertexUV(1 + x, 0 + y, 0 + z, max_u, min_v, 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(0 + x, 0 + y, 1 + z, min_u, min_v, 15);
//		tessellator->vertexUV(0 + x, 1 + y, 1 + z, min_u, max_v, 15);
//		tessellator->vertexUV(1 + x, 1 + y, 0 + z, max_u, max_v, 15);
//		tessellator->vertexUV(1 + x, 0 + y, 0 + z, max_u, min_v, 15);

//		if (m_res->cc_block->additional_data & 1) {
//			auto texture = GetTexture(mb_cfg, ItemId::Snow, 0);
//			tessellator->setTile(getTile(mb_cfg, texture));
			bounds.set(0, 0, 0, 1, 0.125f, 1);
			tessellateBlockInWorld(tile, x, y, z);
//		}
	}

	inline void tessellatePistonBaseInWorld(PistonBaseTile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);

		tile->getVisualShape(1 | 8, x, y, z, bounds);
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellatePistonArmInWorld(PistonArmTile* tile, int x, int y, int z) {
//		float fx = x - 0.5f;
//		float fy = y - 0.5f;
//		float fz = z - 0.5f;

		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);

		int data = 1;
		int rotation = PistonArmTile::getRotation(data);
		bool powered = true;
		float var8 = 0.25F;
		float var9 = 0.375F;
		float var10 = 0.625F;
		float var11 = powered ? 1.0F : 0.5F;
		float var12 = powered ? 16.0F : 8.0F;

		tile->getVisualShape(data, x, y, z, bounds);
//		tessellateBlockInWorld(tile, x, y, z);

		switch(rotation) {
		case 0:
//			rotEast = 3;
//			rotWest = 3;
//			rotSouth = 3;
//			rotNorth = 3;
			tessellateBlockInWorld(tile, x, y, z);///*pos*/);
			tessellatePistonRodUD(tile, x + 0.375F, x + 0.625F, y + 0.25F, y + 0.25F + var11, z + 0.625F, z + 0.625F, 0.8F, var12);
			tessellatePistonRodUD(tile, x + 0.625F, x + 0.375F, y + 0.25F, y + 0.25F + var11, z + 0.375F, z + 0.375F, 0.8F, var12);
			tessellatePistonRodUD(tile, x + 0.375F, x + 0.375F, y + 0.25F, y + 0.25F + var11, z + 0.375F, z + 0.625F, 0.6F, var12);
			tessellatePistonRodUD(tile, x + 0.625F, x + 0.625F, y + 0.25F, y + 0.25F + var11, z + 0.625F, z + 0.375F, 0.6F, var12);
			break;
		case 1:
			tessellateBlockInWorld(tile, x, y, z);//pos);
			tessellatePistonRodUD(tile, x + 0.375F, x + 0.625F, y - 0.25F + 1.0F - var11, y - 0.25F + 1.0F, z + 0.625F, z + 0.625F, 0.8F, var12);
			tessellatePistonRodUD(tile, x + 0.625F, x + 0.375F, y - 0.25F + 1.0F - var11, y - 0.25F + 1.0F, z + 0.375F, z + 0.375F, 0.8F, var12);
			tessellatePistonRodUD(tile, x + 0.375F, x + 0.375F, y - 0.25F + 1.0F - var11, y - 0.25F + 1.0F, z + 0.375F, z + 0.625F, 0.6F, var12);
			tessellatePistonRodUD(tile, x + 0.625F, x + 0.625F, y - 0.25F + 1.0F - var11, y - 0.25F + 1.0F, z + 0.625F, z + 0.375F, 0.6F, var12);
			break;
		case 2:
//			rotSouth = 1;
//			rotNorth = 2;
			tessellateBlockInWorld(tile, x, y, z);//pos);
			tessellatePistonRodSN(tile, x + 0.375F, x + 0.375F, y + 0.625F, y + 0.375F, z + 0.25F, z + 0.25F + var11, 0.6F, var12);
			tessellatePistonRodSN(tile, x + 0.625F, x + 0.625F, y + 0.375F, y + 0.625F, z + 0.25F, z + 0.25F + var11, 0.6F, var12);
			tessellatePistonRodSN(tile, x + 0.375F, x + 0.625F, y + 0.375F, y + 0.375F, z + 0.25F, z + 0.25F + var11, 0.5F, var12);
			tessellatePistonRodSN(tile, x + 0.625F, x + 0.375F, y + 0.625F, y + 0.625F, z + 0.25F, z + 0.25F + var11, 1.0F, var12);
			break;
		case 3:
//			rotSouth = 2;
//			rotNorth = 1;
//			rotTop = 3;
//			rotBottom = 3;
			tessellateBlockInWorld(tile, x, y, z);//pos);
			tessellatePistonRodSN(tile, x + 0.375F, x + 0.375F, y + 0.625F, y + 0.375F, z - 0.25F + 1.0F - var11, z - 0.25F + 1.0F, 0.6F, var12);
			tessellatePistonRodSN(tile, x + 0.625F, x + 0.625F, y + 0.375F, y + 0.625F, z - 0.25F + 1.0F - var11, z - 0.25F + 1.0F, 0.6F, var12);
			tessellatePistonRodSN(tile, x + 0.375F, x + 0.625F, y + 0.375F, y + 0.375F, z - 0.25F + 1.0F - var11, z - 0.25F + 1.0F, 0.5F, var12);
			tessellatePistonRodSN(tile, x + 0.625F, x + 0.375F, y + 0.625F, y + 0.625F, z - 0.25F + 1.0F - var11, z - 0.25F + 1.0F, 1.0F, var12);
			break;
		case 4:
//			rotEast = 1;
//			rotWest = 2;
//			rotTop = 2;
//			rotBottom = 1;
			tessellateBlockInWorld(tile, x, y, z);//pos);
			tessellatePistonRodEW(tile, x + 0.25F, x + 0.25F + var11, y + 0.375F, y + 0.375F, z + 0.625F, z + 0.375F, 0.5F, var12);
			tessellatePistonRodEW(tile, x + 0.25F, x + 0.25F + var11, y + 0.625F, y + 0.625F, z + 0.375F, z + 0.625F, 1.0F, var12);
			tessellatePistonRodEW(tile, x + 0.25F, x + 0.25F + var11, y + 0.375F, y + 0.625F, z + 0.375F, z + 0.375F, 0.6F, var12);
			tessellatePistonRodEW(tile, x + 0.25F, x + 0.25F + var11, y + 0.625F, y + 0.375F, z + 0.625F, z + 0.625F, 0.6F, var12);
			break;
		case 5:
//			rotEast = 2;
//			rotWest = 1;
//			rotTop = 1;
//			rotBottom = 2;
			tessellateBlockInWorld(tile, x, y, z);//pos);
			tessellatePistonRodEW(tile, x - 0.25F + 1.0F - var11, x - 0.25F + 1.0F, y + 0.375F, y + 0.375F, z + 0.625F, z + 0.375F, 0.5F, var12);
			tessellatePistonRodEW(tile, x - 0.25F + 1.0F - var11, x - 0.25F + 1.0F, y + 0.625F, y + 0.625F, z + 0.375F, z + 0.625F, 1.0F, var12);
			tessellatePistonRodEW(tile, x - 0.25F + 1.0F - var11, x - 0.25F + 1.0F, y + 0.375F, y + 0.625F, z + 0.375F, z + 0.375F, 0.6F, var12);
			tessellatePistonRodEW(tile, x - 0.25F + 1.0F - var11, x - 0.25F + 1.0F, y + 0.625F, y + 0.375F, z + 0.625F, z + 0.625F, 0.6F, var12);
		}
	}

	inline void tessellateLanternInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF, 0xFF);

		setRenderBox(2.5f, 0.25, 2.5f, 13.5f, 12.5f, 13.5f);
		tessellateBlockInWorld(tile, x, y, z);

		setRenderBox(4.5f, 12.5f, 4.5f, 11.5f, 14.5, 11.5f);
		tessellateBlockInWorld(tile, x, y, z);

		setRenderBox(7.5f, 14.5, 7.5f, 8.5, 1, 8.5);
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateHopperInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);

		setRenderBox(0, 10, 0, 16, 16, 2);
		tessellateBlockInWorld(tile, x, y, z);

		setRenderBox(0, 10, 14, 16, 16, 16);
		tessellateBlockInWorld(tile, x, y, z);

		setRenderBox(14, 10, 2, 16, 16, 14);
		tessellateBlockInWorld(tile, x, y, z);

		setRenderBox(0, 10, 2, 2, 16, 14);
		tessellateBlockInWorld(tile, x, y, z);

		TextureUVCoordinateSet hopper{0, 0, 1, 1};

		const float coords[8]{
			hopper.getInterpolatedU(0.375f), hopper.getInterpolatedV(0.125f),
			hopper.getInterpolatedU(0.125f), hopper.getInterpolatedV(0.625f),
			hopper.getInterpolatedU(0.875f), hopper.getInterpolatedV(0.625f),
			hopper.getInterpolatedU(0.625f), hopper.getInterpolatedV(0.125f)
		};

		// South
		tessellator->quad();
		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.375f, coords[0], coords[1], 15);
		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.125f, coords[2], coords[3], 15);
		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.125f, coords[4], coords[5], 15);
		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.375f, coords[6], coords[7], 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.375f, coords[0], coords[1], 15);
//		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.125f, coords[2], coords[3], 15);
//		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.125f, coords[4], coords[5], 15);
//		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.375f, coords[6], coords[7], 15);

		// East
		tessellator->quad();
		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.375f, coords[0], coords[1], 15);
		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.125f, coords[2], coords[3], 15);
		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.875f, coords[4], coords[5], 15);
		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.625f, coords[6], coords[7], 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.375f, coords[0], coords[1], 15);
//		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.125f, coords[2], coords[3], 15);
//		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.875f, coords[4], coords[5], 15);
//		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.625f, coords[6], coords[7], 15);

		// North
		tessellator->quad();
		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.625f, coords[0], coords[1], 15);
		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.875f, coords[2], coords[3], 15);
		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.875f, coords[4], coords[5], 15);
		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.625f, coords[6], coords[7], 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(x + 0.625f, y + 0.125f, z + 0.625f, coords[0], coords[1], 15);
//		tessellator->vertexUV(x + 0.875f, y + 0.625f, z + 0.875f, coords[2], coords[3], 15);
//		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.875f, coords[4], coords[5], 15);
//		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.625f, coords[6], coords[7], 15);

		// West
		tessellator->quad();
		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.625f, coords[0], coords[1], 15);
		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.875f, coords[2], coords[3], 15);
		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.125f, coords[4], coords[5], 15);
		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.375f, coords[6], coords[7], 15);

//		tessellator->quadInv();
//		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.625f, coords[0], coords[1], 15);
//		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.875f, coords[2], coords[3], 15);
//		tessellator->vertexUV(x + 0.125f, y + 0.625f, z + 0.125f, coords[4], coords[5], 15);
//		tessellator->vertexUV(x + 0.375f, y + 0.125f, z + 0.375f, coords[6], coords[7], 15);

//		switch (m_res->cc_block->overlay_direction) {
//		case Direction::South:
//			bounds.set(6, 0, 0, 10, 2, 10);
//			break;
//		case Direction::North:
//			bounds.set(6, 0, 6, 10, 2, 16);
//			break;
//		case Direction::East:
//			bounds.set(6, 0, 6, 16, 2, 10);
//			break;
//		case Direction::West:
//			bounds.set(0, 0, 6, 10, 2, 10);
//			break;
//		default:
			bounds.set(6, 0, 6, 10, 2, 10);
//			break;
//		}
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateItemFrameInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);

		int rotation = 2;
		setRotatedBox(rotation, 2, 2, 15.5, 14, 14, 16);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 3, 13, 15, 14, 14, 15.5);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 2, 2, 15, 13, 3, 15.5);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 13, 2, 15, 14, 13, 15.5);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(rotation, 2, 3, 15, 3, 14, 15.5);
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateFireInWorld(Tile* tile, int x, int y, int z) {
		TextureUVCoordinateSet fire{0, 0, 1, 1};

		tessellator->color(0xFF, 0xFF, 0xFF);

		FlammableBit flammable = FlammableBit(~0);

		const float coords[4]{
			fire.getInterpolatedU(0),
			fire.getInterpolatedV(0),
			fire.getInterpolatedU(1),
			fire.getInterpolatedV(1)
		};

		if (flammable && FlammableBit::Up) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0.95, z + 1, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0, y + 0.95, z + 0, coords[2], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 0.65, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0.65, z + 1, coords[0], coords[3], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0.65, z + 1, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 0.65, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0.95, z + 0, coords[2], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 0.95, z + 1, coords[0], coords[1], 15);
		}

		if (flammable && FlammableBit::Down) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0.05, z + 1, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0, y + 0.05, z + 0, coords[2], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 0.35, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0.35, z + 1, coords[0], coords[3], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0.35, z + 1, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 0.35, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0.05, z + 0, coords[2], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 0.05, z + 1, coords[0], coords[1], 15);
		}

		if (flammable && FlammableBit::North) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 1, y + 0, z + 0.95, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 1, z + 0.60, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 1, z + 0.60, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 0, z + 0.95, coords[2], coords[1], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 1, y + 0, z + 0.85, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 1, y + 1, z + 0.75, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 1, z + 0.75, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0, y + 0, z + 0.85, coords[2], coords[1], 15);
		}

		if (flammable && FlammableBit::South) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0, z + 0.05, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0, y + 1, z + 0.60, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 1, z + 0.40, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0, z + 0.05, coords[2], coords[1], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0, y + 0, z + 0.15, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0, y + 1, z + 0.25, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 1, z + 0.25, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 1, y + 0, z + 0.15, coords[2], coords[1], 15);
		}

		if (flammable && FlammableBit::East) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0.95, y + 0, z + 0, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0.60, y + 1, z + 0, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0.60, y + 1, z + 1, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0.95, y + 0, z + 1, coords[2], coords[1], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0.85, y + 0, z + 0, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0.75, y + 1, z + 0, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0.75, y + 1, z + 1, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0.85, y + 0, z + 1, coords[2], coords[1], 15);
		}

		if (flammable && FlammableBit::West) {
			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0.05, y + 0, z + 1, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0.40, y + 1, z + 1, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0.40, y + 1, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0.05, y + 0, z + 0, coords[2], coords[1], 15);

			tessellator->quad();
//			tessellator->quadInv();
			tessellator->vertexUV(x + 0.15, y + 0, z + 1, coords[0], coords[1], 15);
			tessellator->vertexUV(x + 0.25, y + 1, z + 1, coords[0], coords[3], 15);
			tessellator->vertexUV(x + 0.25, y + 1, z + 0, coords[2], coords[3], 15);
			tessellator->vertexUV(x + 0.15, y + 0, z + 0, coords[2], coords[1], 15);
		}
	}

	inline void tessellateGlassPaneInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);

		enum GlassPaneConnection {
			West = 1,
			North = 2,
			East = 4,
			South = 8,

			EW = East | West,
			SN = South | North
		};

		int directions = GlassPaneConnection::SN | GlassPaneConnection::West;// /*m_res->cc_block->additional_data*/0 & 0xF;
		if (!directions) directions = 15;

		if ((directions & GlassPaneConnection::EW) == GlassPaneConnection::EW) {
			directions ^= GlassPaneConnection::EW;
			bounds.set(0, 0, 0.45f, 1, 0.9999f, 0.55f);
			tessellateBlockInWorld(tile, x, y, z);
		}
		if ((directions & GlassPaneConnection::SN) == GlassPaneConnection::SN) {
			directions ^= 0x0A;
			bounds.set(0.45f, 0, 0, 0.55f, 0.9999f, 1);
			tessellateBlockInWorld(tile, x, y, z);
		}

		// west
		if (directions & GlassPaneConnection::West) {
			bounds.set(0, 0, 0.45f, 0.5f, 0.9999f, 0.55f);
			tessellateBlockInWorld(tile, x, y, z);
		}

		if (directions & GlassPaneConnection::North) {
			bounds.set(0.45f, 0, 0.5f, 0.55f, 0.9999f, 1);
			tessellateBlockInWorld(tile, x, y, z);
		}

		if (directions & GlassPaneConnection::East) {
			bounds.set(0.5f, 0, 0.45f, 1, 0.9999f, 0.55f);
			tessellateBlockInWorld(tile, x, y, z);
		}

		if (directions & GlassPaneConnection::South) {
			bounds.set(0.45f, 0, 0, 0.55f, 0.9999f, 0.5f);
			tessellateBlockInWorld(tile, x, y, z);
		}
	}

	inline void tessellateSignInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);
//		tessellator->setTile(getTile(mb_cfg, texture));

		bounds.set(0, 0.4, 0.45, 1, 1, 0.55);
		tessellateBlockInWorld(tile, x, y, z);

		bounds.set(0.45, 0, 0.45, 0.55, 0.4, 0.55);
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateWallSignInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);
//		tessellator->setTile(getTile(mb_cfg, texture));

		int rotation = 2;
		setRotatedBounds(rotation, 0, 0.3, 0.9, 1, 0.9, 1);
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateSlabInWorld(Tile* tile, int x, int y, int z) {
		static constexpr AABB visualShape[] {
			{ 0, 0, 0, 1, 0.5, 1 },
			{ 0, 0.5, 0, 1, 1, 1 }
		};

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setTile(getTile(mb_cfg, texture));
//		tessellator->setMaterial(material);

		bounds = visualShape[/*m_res->cc_block->additional_data*/0];
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateVerticalSlabInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setTile(getTile(mb_cfg, texture));
//		tessellator->setMaterial(material);

		bounds.set(0, 0, 0.5, 1, 1, 1);
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateMiddleSlabInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setTile(getTile(mb_cfg, texture));
//		tessellator->setMaterial(material);

   		bounds.set(0, 0.25, 0, 1, 0.75, 1);
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateTrapDoorInWorld(Tile* tile, int x, int y, int z) {
		static constexpr AABB visualShapes[]{
			{ 0, 0.8125, 0, 1, 1, 1 },
			{ 0, 0, 0.8125, 1, 1, 1 },
			{ 0, 0, 0, 1, 0.1875, 1 },
			{ 0, 0, 0.8125, 1, 1, 1 }
		};

		tessellator->color(0xFF, 0xFF, 0xFF);
//		tessellator->setMaterial(material);
//		tessellator->setTile(getTile(mb_cfg, texture));

		int rotation = 2;
		setRotatedBounds(rotation, visualShapes[0]);
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateSlopeInWorld(Tile* tile, int x, int y, int z) {
   		const float coords[4]{ 0, 0, 1, 1 };

		tessellator->color(0xFF, 0xFF, 0xFF);

		bounds.set(0, 0, 0, 1, 1, 1);

        float p1[] { bounds.minX, bounds.minY, bounds.minZ };
	 	float p2[] { bounds.minX, bounds.minY, bounds.maxZ };
        float p3[] { bounds.maxX, bounds.minY, bounds.maxZ };
        float p4[] { bounds.maxX, bounds.minY, bounds.minZ };

		float p5[] { bounds.minX, bounds.maxY, bounds.minZ };
        float p6[] { bounds.minX, bounds.maxY, bounds.maxZ };
        float p7[] { bounds.maxX, bounds.maxY, bounds.maxZ };
        float p8[] { bounds.maxX, bounds.maxY, bounds.minZ };

		bool flip0 = false;
		bool flip1 = false;
		bool flip2 = false;
		bool flip3 = false;
		bool flip4 = false;
		bool flip5 = false;

		int data = 0;
        switch (data) {
		case 0:
			flip5 = true;
			p5[1] = 0;
			p8[1] = 0;
			break;
		case 1:
			flip2 = true;
			p5[1] = 0;
			p6[1] = 0;
			break;
		case 2:
			flip4 = true;
			p6[1] = 0;
			p7[1] = 0;
			break;
		case 3:
			flip3 = true;
			p7[1] = 0;
			p8[1] = 0;
			break;
		case 4:
			flip4 = true;
			p1[1] = 1;
			p4[1] = 1;
			break;
		case 5:
			flip3 = true;
			p1[1] = 1;
			p2[1] = 1;
			break;
		case 6:
			flip5 = true;
			p2[1] = 1;
			p3[1] = 1;
			break;
		case 7:
			flip2 = true;
			p3[1] = 1;
			p4[1] = 1;
			break;
		case 8:
			flip0 = true;
			p1[2] = 1;
			p5[2] = 1;
			break;
		case 9:
			flip1 = true;
			p2[0] = 1;
			p6[0] = 1;
			break;
		case 10:
			flip0 = true;
			p3[2] = 0;
			p7[2] = 0;
			break;
		case 11:
			flip1 = true;
			p4[0] = 0;
			p8[0] = 0;
			break;
		}

		tessellator->normal(0, 1, 0);
		tessellator->quad(0, 1, flip0 ? 3 : 2, flip0 ? 3 : 0, flip0 ? 1 : 2, flip0 ? 2 : 3);
		face(x, y, z, p5, p6, p7, p8, coords);

		tessellator->normal(0, -1, 0);
		tessellator->quad(0, 1, flip1 ? 3 : 2, flip1 ? 3 : 0, flip1 ? 1 : 2, flip1 ? 2 : 3);
		face(x, y, z, p2, p1, p4, p3, coords);

		tessellator->normal(0, 0, 1);
		tessellator->quad(0, 1, flip2 ? 3 : 2, flip2 ? 3 : 0, flip2 ? 1 : 2, flip2 ? 2 : 3);
		face(x, y, z, p3, p7, p6, p2, coords);

		tessellator->normal(0, 0, -1);
		tessellator->quad(0, 1, flip3 ? 3 : 2, flip3 ? 3 : 0, flip3 ? 1 : 2, flip3 ? 2 : 3);
		face(x, y, z, p1, p5, p8, p4, coords);

		tessellator->normal(1, 0, 0);
		tessellator->quad(0, 1, flip4 ? 3 : 2, flip4 ? 3 : 0, flip4 ? 1 : 2, flip4 ? 2 : 3);
		face(x, y, z, p4, p8, p7, p3, coords);

		tessellator->normal(-1, 0, 0);
		tessellator->quad(0, 1, flip5 ? 3 : 2, flip5 ? 3 : 0, flip5 ? 1 : 2, flip5 ? 2 : 3);
		face(x, y, z, p2, p6, p5, p1, coords);
	}

	inline void tessellateCakeInWorld(Tile* tile, int x, int y, int z) {
        setRenderBox(1, 0, 1, 15/* - decor*/, 10, 15);
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateCocoaInWorld(Tile* tile, int x, int y, int z) {
		int data = 0;

		int rot = data & 3;
		int age = (data >> 2) & 2;

		tessellator->color(0xFF, 0xFF, 0xFF);

		switch (age) {
		case 0:
			setRenderBox(6, 6.5, 10.75, 10, 12, 15);
			break;
		case 1:
			setRenderBox(5.5, 4, 9, 10.5, 12, 15);
			break;
		case 2:
			setRenderBox(5, 0.75, 7, 11, 12, 15);
			break;
		}
		tessellateBlockInWorld(tile, x, y, z);

		switch (age) {
		case 0:
			setRenderBox(8, 12, 13, 8, 16, 16);
			break;
		case 1:
			setRenderBox(8, 12, 12.5, 8, 16, 16);
			break;
		case 2:
			setRenderBox(8, 12, 9.75, 8, 16, 16);
			break;
		}
		tessellateBlockInWorld(tile, x, y, z);
	}

	inline void tessellateCampfireInWorld(Tile* tile, int x, int y, int z) {
		tessellator->color(0xFF, 0xFF, 0xFF);

		setRenderBox(3, 0, 3, 16 - 3, 1, 16 - 3);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(2, 3, 0, 0, 16, 4, 3);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(4, 3, 0, 0, 16, 4, 3);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(3, 3, 0, 0, 16, 4, 3);
		tessellateBlockInWorld(tile, x, y, z);

		setRotatedBox(5, 3, 0, 0, 16, 4, 3);
		tessellateBlockInWorld(tile, x, y, z);

		ModelTransform transform0 {
			.origin = {0.5f, 0.0f, 0},
			.rotX = glm::radians(15.0f)
		};
		transform = &transform0;

		setRotatedBox(2, 8 - 1.5, 1.5, 2, 8 + 1.5, 1.5 + 14, 2 + 2);
		tessellateBlockInWorld(tile, x, y, z);

		ModelTransform transform1 {
			.origin = {0.5f, 0.0f, 1},
			.rotX = glm::radians(-15.0f)
		};
		transform = &transform1;

		setRotatedBox(3, 8 - 1.5, 1.5, 2, 8 + 1.5, 1.5 + 14, 2 + 2);
		tessellateBlockInWorld(tile, x, y, z);

		ModelTransform transform2 {
			.origin = {1.0f, 0.0f, 0.5f},
			.rotZ = glm::radians(9.5f)
		};
		transform = &transform2;

		setRotatedBox(4, 8 - 1.5, 1.5, 2, 8 + 1.5, 1.5 + 14, 2 + 2);
		tessellateBlockInWorld(tile, x, y, z);

		ModelTransform transform3 {
			.origin = {0.0f, 0.0f, 0.5f},
			.rotZ = glm::radians(-9.5f)
		};
		transform = &transform3;

		setRotatedBox(5, 8 - 1.5, 1.5, 2, 8 + 1.5, 1.5 + 14, 2 + 2);
		tessellateBlockInWorld(tile, x, y, z);

		transform = nullptr;
	}
private:
	void tessellatePistonRodUD(PistonArmTile* tile, float minX, float maxX, float minY, float maxY, float minZ, float maxZ, float a, float b) {
		TextureUVCoordinateSet piston = {0, 0, 1, 1};
		//TextureUVCoordinateSet(0.6875F, 0.375F, 0.7187F, 0.4375F, 16, 16);

		float minU = piston.minU;
		float minV = piston.minV;
		float maxU = piston.getInterpolatedU(1);//b);
		float maxV = piston.getInterpolatedV(1);//4.0);
		tessellator->quad();
		tessellator->color(0xFF, 0xFF, 0xFF);
		tessellator->vertexUV(minX, maxY, minZ, maxU, minV);
		tessellator->vertexUV(minX, minY, minZ, minU, minV);
		tessellator->vertexUV(maxX, minY, maxZ, minU, maxV);
		tessellator->vertexUV(maxX, maxY, maxZ, maxU, maxV);
	}

	void tessellatePistonRodSN(PistonArmTile* tile, float x, float xx, float y, float yy, float z, float zz, float a, float b) {
		TextureUVCoordinateSet piston = {0, 0, 1, 1};

		float var18 = piston.minU;
		float var20 = piston.minV;
		float var22 = piston.getInterpolatedU(1);//b);
		float var24 = piston.getInterpolatedV(1);//4.0);
		tessellator->quad();
		tessellator->color(0xFF, 0xFF, 0xFF);//a, a, a, 1.0F);
		tessellator->vertexUV(x, y, zz, var22, var20);
		tessellator->vertexUV(x, y, z, var18, var20);
		tessellator->vertexUV(xx, yy, z, var18, var24);
		tessellator->vertexUV(xx, yy, zz, var22, var24);
	}

	void tessellatePistonRodEW(PistonArmTile* tile, float x, float xx, float y, float yy, float z, float zz, float a, float b) {
		TextureUVCoordinateSet piston = {0, 0, 1, 1};
//		TextureUVCoordinateSet piston = TextureUVCoordinateSet(0.6875F, 0.375F, 0.7187F, 0.4375F, 16, 16);

		float var18 = piston.minU;
		float var20 = piston.minV;
		float var22 = piston.getInterpolatedU(1);//b);
		float var24 = piston.getInterpolatedV(1);//4.0);
		tessellator->quad();
		tessellator->color(0xFF, 0xFF, 0xFF);//a, a, a, 1.0F);
		tessellator->vertexUV(xx, y, z, var22, var20);
		tessellator->vertexUV(x, y, z, var18, var20);
		tessellator->vertexUV(x, yy, zz, var18, var24);
		tessellator->vertexUV(xx, yy, zz, var22, var24);
	}

	inline void tessellateUp(Tile* tile, float x, float y, float z) {
//        if (!useForcedUV) {
            float minU = bounds.minX;
            float minV = bounds.minZ;
            float maxU = bounds.maxX;
            float maxV = bounds.maxZ;
//        }

		const auto texture = useForcedUV ? forcedUV : tile->getTexture(0, 0);
        const float coords[4]{
            texture.getInterpolatedU(minU),
            texture.getInterpolatedV(minV),
            texture.getInterpolatedU(maxU),
            texture.getInterpolatedV(maxV)
        };

        float p1[] { bounds.minX, bounds.maxY, bounds.minZ };
        float p2[] { bounds.minX, bounds.maxY, bounds.maxZ };
        float p3[] { bounds.maxX, bounds.maxY, bounds.maxZ };
        float p4[] { bounds.maxX, bounds.maxY, bounds.minZ };

        apply(transform, p1, p2, p3, p4);

    	tessellator->normal(0, 1, 0);
    	face(x, y, z, p1, p2, p3, p4, coords);
    }

	inline void face(int x, int y, int z, float p1[4], float p2[4], float p3[4], float p4[4], const float (&coords)[4]) {
//        tessellator->quad();
        tessellator->vertexUV(p1[0] + x, p1[1] + y, p1[2] + z, coords[0], coords[1], 15);
        tessellator->vertexUV(p2[0] + x, p2[1] + y, p2[2] + z, coords[0], coords[3], 15);
        tessellator->vertexUV(p3[0] + x, p3[1] + y, p3[2] + z, coords[2], coords[3], 15);
        tessellator->vertexUV(p4[0] + x, p4[1] + y, p4[2] + z, coords[2], coords[1], 15);
	}

	inline void apply(ModelTransform* modelTransform, float p1[4], float p2[4], float p3[4], float p4[4]) {
		for (auto it = modelTransform; it != nullptr; it = it->next) {
			it->apply(p1, p2, p3, p4);
		}
	}
};

struct AgentEntity {
	ModelRenderer agentModel;

	AgentEntity(ResourceManager* resourceManager, GameRenderer* renderer) {
		vk::SamplerCreateInfo samplerCreateInfo{
				.magFilter = vk::Filter::eNearest,
				.minFilter = vk::Filter::eNearest,
				.mipmapMode = vk::SamplerMipmapMode::eNearest,
				.addressModeU = vk::SamplerAddressMode::eRepeat,
				.addressModeV = vk::SamplerAddressMode::eRepeat,
				.addressModeW = vk::SamplerAddressMode::eRepeat,
				.maxAnisotropy = 0,
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

			auto mobs = parser.parse().value();

			auto mob = mobs.get("geometry.agent");
			{
				int texture_width = mob->get("texturewidth")->as_i64().value();
				int texture_height = mob->get("textureheight")->as_i64().value();

				if (auto bones = mob->get("bones")) {
					for (auto& bone : bones->as_array().value()) {
						if (auto cubes = bone.get("cubes")) {
							for (auto& cube : cubes->as_array().value()) {
								auto& origin = cube.get("origin").value().as_array().value();
								auto& size = cube.get("size").value().as_array().value();
								auto& uv = cube.get("uv").value().as_array().value();

//								if (!neverRender) {
									const float posX = origin[0].as_f64().value() / 16.0f;
									const float posY = origin[1].as_f64().value() / 16.0f;
									const float posZ = origin[2].as_f64().value() / 16.0f;

									const float sizeX = size[0].as_f64().value();
									const float sizeY = size[1].as_f64().value();
									const float sizeZ = size[2].as_f64().value();

									const float x0 = posX;
									const float y0 = posY;
									const float z0 = posZ;

									const float x1 = posX + sizeX / 16.0f;
									const float y1 = posY + sizeY / 16.0f;
									const float z1 = posZ + sizeZ / 16.0f;

									const float u = uv[0].as_f64().value();
									const float v = uv[1].as_f64().value();

									const float u0 = (u) / texture_width;
									const float u1 = (u + sizeZ) / texture_width;
									const float u2 = (u + sizeZ + sizeX) / texture_width;
									const float u3 = (u + sizeZ + sizeX + sizeX) / texture_width;
									const float u4 = (u + sizeZ + sizeX + sizeZ) / texture_width;
									const float u5 = (u + sizeZ + sizeX + sizeZ + sizeX) / texture_width;

									const float v0 = (v) / texture_height;
									const float v1 = (v + sizeZ) / texture_height;
									const float v2 = (v + sizeZ + sizeY) / texture_height;

									TexturedQuad quad0 {
									    .vertices{
									        PositionTextureVertex{x0, y0, z0, u1, v2},
									        PositionTextureVertex{x0, y1, z0, u1, v1},
									        PositionTextureVertex{x1, y1, z0, u2, v1},
									        PositionTextureVertex{x1, y0, z0, u2, v2}
                                        },
									    .normal{0, 0, -1.0f}
                                    };
									TexturedQuad quad1 {
                                        .vertices{
                                            PositionTextureVertex{x1, y0, z0, u1, v2},
                                            PositionTextureVertex{x1, y1, z0, u1, v1},
                                            PositionTextureVertex{x1, y1, z1, u0, v1},
                                            PositionTextureVertex{x1, y0, z1, u0, v2}
                                        },
                                        .normal{1.0f, 0, 0}
									};
									TexturedQuad quad2 {
                                        .vertices{
                                            PositionTextureVertex{x1, y0, z1, u4, v2},
                                            PositionTextureVertex{x1, y1, z1, u4, v1},
                                            PositionTextureVertex{x0, y1, z1, u5, v1},
                                            PositionTextureVertex{x0, y0, z1, u5, v2}
                                        },
                                        .normal{0, 0, 1.0f}
									};
									TexturedQuad quad3 {
                                        .vertices{
                                            PositionTextureVertex{x0, y0, z1, u0, v2},
                                            PositionTextureVertex{x0, y1, z1, u0, v1},
                                            PositionTextureVertex{x0, y1, z0, u1, v1},
                                            PositionTextureVertex{x0, y0, z0, u1, v2}
                                        },
                                        .normal{-1.0f, 0, 0}
									};
									TexturedQuad quad4 {
                                        .vertices{
                                            PositionTextureVertex{x0, y1, z0, u1, v1},
                                            PositionTextureVertex{x0, y1, z1, u1, v0},
                                            PositionTextureVertex{x1, y1, z1, u2, v0},
                                            PositionTextureVertex{x1, y1, z0, u2, v1}
                                        },
                                        .normal{0, 1.0f, 0}
									};
									TexturedQuad quad5 {
                                        .vertices{
                                            PositionTextureVertex{x0, y0, z1, u2, v1},
                                            PositionTextureVertex{x0, y0, z0, u2, v0},
                                            PositionTextureVertex{x1, y0, z0, u3, v0},
                                            PositionTextureVertex{x1, y0, z1, u3, v1}
                                        },
                                        .normal{0, -1.0f, 0}
									};

//									TexturedQuad quad0({vertex0, vertex1, vertex2, vertex3}, u1, v1, u2, v2, texture_width, texture_height, {0, 0, -1});
//									TexturedQuad quad1({vertex3, vertex2, vertex4, vertex5}, u1, v1, u0, v2, texture_width, texture_height, {1, 0, 0});
//									TexturedQuad quad2({vertex5, vertex4, vertex6, vertex7}, u4, v1, u5, v2, texture_width, texture_height, {0, 0, 1});
//									TexturedQuad quad3({vertex7, vertex6, vertex1, vertex0}, u0, v1, u1, v2, texture_width, texture_height, {-1, 0, 0});
//									TexturedQuad quad4({vertex1, vertex6, vertex4, vertex2}, u1, v0, u2, v1, texture_width, texture_height, {0, 1, 0});
//									TexturedQuad quad5({vertex7, vertex0, vertex3, vertex5}, u2, v0, u3, v1, texture_width, texture_height, {0, -1, 0});

									ModelBox modelBox{
										.quads {
											quad0,
											quad1,
											quad2,
											quad3,
											quad4,
											quad5
										}
									};

									agentModel.cubes.push_back(modelBox);
//								}
							}
						}
					}
				}
			}
		});

//		for (auto&& cube : agentModel.cubes) {
//			for (auto&& quad : cube.quads) {
//				if (!quad.has_value()) continue;
//				auto normal = quad->normal;
//
//				vertexBuilder.addQuad(0, 1, 2, 0, 2, 3);
//				for (auto&& vertex : quad->vertices) {
//					vertexBuilder.vertices.emplace_back(
//						vertex.x, vertex.y, vertex.z,
//						vertex.u, vertex.v,
//						normal.x, normal.y, normal.z
//					);
//				}
//			}
//		}

		BlockTessellator blockTessellator{};

		auto tessellator = Tessellator::instance();

//		blockTessellator.tessellateButtonInWorld(Tiles::button, 0, 1, 0);

//		tessellator->setOffset(0, 0.3f, 0);
//		blockTessellator.tessellatePistonArmInWorld(Tiles::pistonArm, 0, 0, 0);

//		blockTessellator.rotY = glm::radians(45.0f);

//		blockTessellator.tessellateCocoaInWorld(Tiles::pistonArm, 0, 1, 0);
		blockTessellator.tessellateCampfireInWorld(Tiles::pistonArm, 0, 1, 0);
//		for (int i = 0; i <= 11; i++)
//		blockTessellator.tessellateSlopeInWorld(i, Tiles::pistonArm, i << 1, 1, 0);
//		blockTessellator.tessellateAnvilInWorld(Tiles::pistonArm, 0, 1, 0);
//		blockTessellator.tessellateBrewingStand(Tiles::pistonArm, 1, 1, 0);
//		blockTessellator.tessellateFenceInWorld(Tiles::pistonArm, 2, 1, 0);
//		blockTessellator.tessellateWallInWorld(Tiles::pistonArm, 3, 1, 0);
//		blockTessellator.tessellateFenceGateInWorld(Tiles::pistonArm, 4, 1, 0);
//		blockTessellator.tessellateWallTorchInWorld(Tiles::pistonArm, 5, 1, 0);
//		blockTessellator.tessellatePaintingInWorld(Tiles::pistonArm, 6, 1, 0);
//		blockTessellator.tessellateLilyPadInWorld(Tiles::pistonArm, 7, 1, 0);
//		blockTessellator.tessellateRailInWorld(Tiles::pistonArm, 8, 1, 0);
//		blockTessellator.tessellateFlowerInWorld(Tiles::pistonArm, 9, 1, 0);
//		blockTessellator.tessellateLanterInWorld(Tiles::pistonArm, 10, 1, 0);
//		blockTessellator.tessellateHopperInWorld(Tiles::pistonArm, 11, 1, 0);
//		blockTessellator.tessellateItemFrameInWorld(Tiles::pistonArm, 12, 1, 0);
//		blockTessellator.tessellateFireInWorld(Tiles::pistonArm, 13, 1, 0);
//		blockTessellator.tessellateGlassPaneInWorld(Tiles::pistonArm, 14, 1, 0);
//		blockTessellator.tessellateSignInWorld(Tiles::pistonArm, 15, 1, 0);
//		blockTessellator.tessellateWallSignInWorld(Tiles::pistonArm, 16, 1, 0);
//		blockTessellator.tessellateSlabInWorld(Tiles::pistonArm, 17, 1, 0);
//		blockTessellator.tessellateVerticalSlabInWorld(Tiles::pistonArm, 18, 1, 0);
//		blockTessellator.tessellateMiddleSlabInWorld(Tiles::pistonArm, 19, 1, 0);
//		blockTessellator.tessellateTrapDoorInWorld(Tiles::pistonArm, 20, 1, 0);
//		blockTessellator.tessellateSpawnerInWorld(Tiles::pistonArm, 0, 0, 0);

//		tessellator->setOffset(0, 0, 0);
//		blockTessellator.tessellatePistonBaseInWorld(Tiles::pistonBase, 0, 0, 0);

//		tessellator->setOffset(0, 0, 0);
//		blockTessellator.bounds.set(0, 0, 0, 1, 1, 1);
//		blockTessellator.tessellateBlockInWorld(Tiles::pistonBase, -1, 0, 0);

		auto& tesselator = blockTessellator.tessellator;
		renderBuffer.SetIndexBufferCount(tesselator->indices.size(), sizeof(int));
		renderBuffer.SetVertexBufferCount(tesselator->vertices.size(), sizeof(Vertex));

		renderBuffer.SetIndexBufferData(tesselator->indices.data(), 0, 0, sizeof(int) * tesselator->indices.size());
		renderBuffer.SetVertexBufferData(tesselator->vertices.data(), 0, 0, sizeof(Vertex) * tesselator->vertices.size());
//
//		renderBuffer.SetIndexBufferSize(sizeof(int) * vertexBuilder.indices.size());
//		renderBuffer.SetVertexBufferSize(sizeof(Vertex) * vertexBuilder.vertices.size());
//
//		renderBuffer.SetIndexBufferData(vertexBuilder.indices.data(), 0, 0, sizeof(int) * vertexBuilder.indices.size());
//		renderBuffer.SetVertexBufferData(vertexBuilder.vertices.data(), 0, 0, sizeof(Vertex) * vertexBuilder.vertices.size());
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

		cmd.setViewport(0, 1, &viewport);
		cmd.setScissor(0, 1, &scissor);

		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &texture.descriptor, 0, nullptr);
		cmd.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(CameraTransform), &transform);
		cmd.bindVertexBuffers(0, 1, vertexBuffers, &offset);
		cmd.bindIndexBuffer(renderBuffer.IndexBuffer, 0, vk::IndexType::eUint32);

		cmd.drawIndexed(renderBuffer.IndexCount, 1, 0, 0, 0);
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

	void quit() {
		wantToQuitFlag = true;
	}

	bool wantToQuit() {
		return wantToQuitFlag;
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

	bool wantToQuitFlag{false};
};

struct Resource {

};

struct ResourcePack {
	std::filesystem::path basePath;

	ResourcePack(std::filesystem::path path) : basePath(std::move(path)) {}

	std::filesystem::path getFullPath(const std::filesystem::path& path) {
		return basePath / path;
	}

	template<typename Fn>
	void LoadResources(const std::filesystem::path& path, Fn&& fn) {
		for (auto& entry : std::filesystem::directory_iterator(getFullPath(path))) {
			if (!entry.is_directory()) {
				std::vector<char> bytes(std::filesystem::file_size(entry.path()));

				std::ifstream file(entry.path(), std::ios::binary);
				file.read(bytes.data(), bytes.size());

				fn(bytes);
			}
		}
	}

	template<typename Fn>
	void LoadResource(const std::filesystem::path& path, Fn&& fn) {
		auto fullPath = getFullPath(path);
		std::vector<char> bytes(std::filesystem::file_size(fullPath));

		std::ifstream file(fullPath, std::ios::binary);
		file.read(bytes.data(), bytes.size());

		fn(bytes);
	}

	template<typename Fn>
	void RecursiveLoadResources(const std::filesystem::path& path, Fn&& fn) {
		for (auto& entry : std::filesystem::recursive_directory_iterator(getFullPath(path))) {
			if (!entry.is_directory()) {
				std::vector<char> bytes(std::filesystem::file_size(entry.path()));

				std::ifstream file(entry.path(), std::ios::binary);
				file.read(bytes.data(), bytes.size());

				fn(bytes);
			}
		}
	}
};

struct ModelPartBox {
	std::unordered_map<std::string, TexturedQuad> quads;
};

struct ModelPart {
	std::string name;
	std::string parent;
	bool neverRender = false;

	std::vector<ModelPartBox> cubes;
};

struct ModelGeometry {
	std::string name;
	std::string parent;
	std::optional<int> texture_width;
	std::optional<int> texture_height;

	std::vector<ModelPart> bones;
};

std::unordered_map<std::string, std::unique_ptr<ModelGeometry>> models;

std::vector<std::string> decompose(std::string name, char ch) {
	std::vector<std::string> ret;

	auto delim = name.find_first_of(ch);
	if (delim != name.npos) {
		ret.emplace_back(name.substr(0, delim));
		ret.emplace_back(name.substr(delim + 1));
	} else {
		ret.emplace_back(std::move(name));
	}

	return std::move(ret);
}

std::unordered_map<std::string, TexturedQuad> createQuads_v1_8_0(float posX, float posY, float posZ, float sizeX, float sizeY, float sizeZ, float u, float v) {
	const float x0 = posX / 16.0f;
	const float y0 = posY / 16.0f;
	const float z0 = posZ / 16.0f;

	const float x1 = (posX + sizeX) / 16.0f;
	const float y1 = (posY + sizeY) / 16.0f;
	const float z1 = (posZ + sizeZ) / 16.0f;

	const float u0 = u;
	const float u1 = u + sizeZ;
	const float u2 = u + sizeZ + sizeX;
	const float u3 = u + sizeZ + sizeX + sizeX;
	const float u4 = u + sizeZ + sizeX + sizeZ;
	const float u5 = u + sizeZ + sizeX + sizeZ + sizeX;

	const float v0 = v;
	const float v1 = v + sizeZ;
	const float v2 = v + sizeZ + sizeY;

	std::unordered_map<std::string, TexturedQuad> quads;
	quads.emplace("south", TexturedQuad {
		.vertices{
			PositionTextureVertex{x0, y0, z0, u1, v2},
			PositionTextureVertex{x0, y1, z0, u1, v1},
			PositionTextureVertex{x1, y1, z0, u2, v1},
			PositionTextureVertex{x1, y0, z0, u2, v2}
		},
		.normal{0, 0, -1.0f}
	});

	quads.emplace("east", TexturedQuad {
		.vertices{
			PositionTextureVertex{x1, y0, z0, u1, v2},
			PositionTextureVertex{x1, y1, z0, u1, v1},
			PositionTextureVertex{x1, y1, z1, u0, v1},
			PositionTextureVertex{x1, y0, z1, u0, v2}
		},
		.normal{1.0f, 0, 0}
	});
	quads.emplace("north", TexturedQuad {
		.vertices{
			PositionTextureVertex{x1, y0, z1, u4, v2},
			PositionTextureVertex{x1, y1, z1, u4, v1},
			PositionTextureVertex{x0, y1, z1, u5, v1},
			PositionTextureVertex{x0, y0, z1, u5, v2}
		},
		.normal{0, 0, 1.0f}
	});
	quads.emplace("west", TexturedQuad {
		.vertices{
			PositionTextureVertex{x0, y0, z1, u0, v2},
			PositionTextureVertex{x0, y1, z1, u0, v1},
			PositionTextureVertex{x0, y1, z0, u1, v1},
			PositionTextureVertex{x0, y0, z0, u1, v2}
		},
		.normal{-1.0f, 0, 0}
	});
	quads.emplace("up", TexturedQuad {
		.vertices{
			PositionTextureVertex{x0, y1, z0, u1, v1},
			PositionTextureVertex{x0, y1, z1, u1, v0},
			PositionTextureVertex{x1, y1, z1, u2, v0},
			PositionTextureVertex{x1, y1, z0, u2, v1}
		},
		.normal{0, 1.0f, 0}
	});
	quads.emplace("down", TexturedQuad {
		.vertices{
			PositionTextureVertex{x0, y0, z1, u2, v1},
			PositionTextureVertex{x0, y0, z0, u2, v0},
			PositionTextureVertex{x1, y0, z0, u3, v0},
			PositionTextureVertex{x1, y0, z1, u3, v1}
		},
		.normal{0, -1.0f, 0}
	});
	return std::move(quads);
}

std::unordered_map<std::string, TexturedQuad> createQuads_v1_12_0(float posX, float posY, float posZ, float sizeX, float sizeY, float sizeZ, json::Object& faces) {
	const float x0 = posX / 16.0f;
	const float y0 = posY / 16.0f;
	const float z0 = posZ / 16.0f;

	const float x1 = (posX + sizeX) / 16.0f;
	const float y1 = (posY + sizeY) / 16.0f;
	const float z1 = (posZ + sizeZ) / 16.0f;

//	const float u0 = u;
//	const float u1 = u + sizeZ;
//	const float u2 = u + sizeZ + sizeX;
//	const float u3 = u + sizeZ + sizeX + sizeX;
//	const float u4 = u + sizeZ + sizeX + sizeZ;
//	const float u5 = u + sizeZ + sizeX + sizeZ + sizeX;
//
//	const float v0 = v;
//	const float v1 = v + sizeZ;
//	const float v2 = v + sizeZ + sizeY;

	std::unordered_map<std::string, TexturedQuad> quads;
	for (auto& [name, face] : faces) {
		auto&& uv = face.get("uv").value().as_array().value();

		float u0 = uv[0].as_f64().value();
		float v0 = uv[1].as_f64().value();

		if (name == "south") {
			auto uv_size = face.get("uv_size").value_or(json::Array{sizeX, sizeY}).as_array().value();

			float u1 = u0 + uv_size[0].as_f64().value();
			float v1 = v0 + uv_size[1].as_f64().value();

			quads.emplace("south", TexturedQuad {
				.vertices{
					PositionTextureVertex{x0, y0, z0, u0, v1},
					PositionTextureVertex{x0, y1, z0, u0, v0},
					PositionTextureVertex{x1, y1, z0, u1, v0},
					PositionTextureVertex{x1, y0, z0, u1, v1}
				},
				.normal{0, 0, -1.0f}
			});
			continue;
		}
		if (name == "east") {
			auto uv_size = face.get("uv_size").value_or(json::Array{sizeZ, sizeY}).as_array().value();

			float u1 = u0 + uv_size[0].as_f64().value();
			float v1 = v0 + uv_size[1].as_f64().value();

			quads.emplace("east", TexturedQuad {
				.vertices{
					PositionTextureVertex{x1, y0, z0, u1, v1},
					PositionTextureVertex{x1, y1, z0, u1, v0},
					PositionTextureVertex{x1, y1, z1, u0, v0},
					PositionTextureVertex{x1, y0, z1, u0, v1}
				},
				.normal{1.0f, 0, 0}
			});
			continue;
		}
		if (name == "north") {
			auto uv_size = face.get("uv_size").value_or(json::Array{sizeX, sizeY}).as_array().value();

			float u1 = u0 + uv_size[0].as_f64().value();
			float v1 = v0 + uv_size[1].as_f64().value();

			quads.emplace("north", TexturedQuad {
				.vertices{
					PositionTextureVertex{x1, y0, z1, u0, v1},
					PositionTextureVertex{x1, y1, z1, u0, v0},
					PositionTextureVertex{x0, y1, z1, u1, v0},
					PositionTextureVertex{x0, y0, z1, u1, v1}
				},
				.normal{0, 0, 1.0f}
			});
			continue;
		}
		if (name == "west") {
			auto uv_size = face.get("uv_size").value_or(json::Array{sizeZ, sizeY}).as_array().value();

			float u1 = u0 + uv_size[0].as_f64().value();
			float v1 = v0 + uv_size[1].as_f64().value();

			quads.emplace("west", TexturedQuad {
				.vertices{
					PositionTextureVertex{x0, y0, z1, u0, v1},
					PositionTextureVertex{x0, y1, z1, u0, v1},
					PositionTextureVertex{x0, y1, z0, u1, v0},
					PositionTextureVertex{x0, y0, z0, u1, v0}
				},
				.normal{-1.0f, 0, 0}
			});
			continue;
		}
		if (name == "up") {
			auto uv_size = face.get("uv_size").value_or(json::Array{sizeX, sizeZ}).as_array().value();

			float u1 = u0 + uv_size[0].as_f64().value();
			float v1 = v0 + uv_size[1].as_f64().value();

			quads.emplace("up", TexturedQuad {
				.vertices{
					PositionTextureVertex{x0, y1, z0, u0, v1},
					PositionTextureVertex{x0, y1, z1, u0, v0},
					PositionTextureVertex{x1, y1, z1, u1, v0},
					PositionTextureVertex{x1, y1, z0, u1, v1}
				},
				.normal{0, 1.0f, 0}
			});
			continue;
		}
		if (name == "down") {
			auto uv_size = face.get("uv_size").value_or(json::Array{sizeX, sizeZ}).as_array().value();

			float u1 = u0 + uv_size[0].as_f64().value();
			float v1 = v0 + uv_size[1].as_f64().value();

			quads.emplace("down", TexturedQuad {
				.vertices{
					PositionTextureVertex{x0, y0, z1, u0, v1},
					PositionTextureVertex{x0, y0, z0, u0, v0},
					PositionTextureVertex{x1, y0, z0, u1, v0},
					PositionTextureVertex{x1, y0, z1, u1, v1}
				},
				.normal{0, -1.0f, 0}
			});
			continue;
		}
	}
	return std::move(quads);
}

void LoadGeometry_v1_8_0(std::span<char> bytes, json::Object& obj) {
	for (auto& [name, geometry] : obj) {
		auto parts = decompose(name, ':');

		ModelGeometry model;

		model.name = std::move(parts[0]);
		if (parts.size() > 1) {
			model.parent = std::move(parts[1]);
		}

		if (auto texture_width = geometry.get("texturewidth")) {
			model.texture_width = texture_width.value().as_i64().value();
		}
		if (auto texture_height = geometry.get("textureheight")) {
			model.texture_width = texture_height.value().as_i64().value();
		}

		if (auto bones = geometry.get("bones")) {
			for (auto& bone : bones->as_array().value()) {
				ModelPart part;
				part.name = bone.get("name").value().as_string().value();

				if (auto parent = bone.get("parent")) {
					part.parent = parent->as_string().value();
				}

				if (auto neverRender = bone.get("neverRender")) {
					part.neverRender = neverRender->as_bool().value();
				}

				if (auto cubes = bone.get("cubes")) {
					for (auto cube : cubes->as_array().value()) {
						auto& origin = cube.get("origin").value().as_array().value();
						auto& size = cube.get("size").value().as_array().value();
						auto uv = cube.get("uv").value_or(json::Array{0, 0}).as_array().value();

						const float posX = origin[0].as_f64().value();
						const float posY = origin[1].as_f64().value();
						const float posZ = origin[2].as_f64().value();

						const float sizeX = size[0].as_f64().value();
						const float sizeY = size[1].as_f64().value();
						const float sizeZ = size[2].as_f64().value();

						const float u = uv[0].as_f64().value();
						const float v = uv[1].as_f64().value();

						ModelPartBox modelBox{
							.quads = createQuads_v1_8_0(posX, posY, posZ, sizeX, sizeY, sizeZ, u, v)
						};

						part.cubes.emplace_back(modelBox);
					}
				}

				model.bones.emplace_back(std::move(part));
			}
		}
	}
}

void LoadGeometry_v1_12_0(std::span<char> bytes, json::Object& obj) {
	for (auto& geometry : obj.at("minecraft:geometry").as_array().value()) {
		auto& description = geometry.get("description").value();

		ModelGeometry model;
		model.name = description.get("identifier").value().as_string().value();

		if (auto texture_width = description.get("texture_width")) {
			model.texture_width = texture_width.value().as_i64().value();
		}
		if (auto texture_height = description.get("texture_height")) {
			model.texture_width = texture_height.value().as_i64().value();
		}

		if (auto bones = geometry.get("bones")) {
			for (auto& bone : bones->as_array().value()) {
				ModelPart part;
				part.name = bone.get("name").value().as_string().value();

				if (auto parent = bone.get("parent")) {
					part.parent = parent->as_string().value();
				}

				if (auto neverRender = bone.get("neverRender")) {
					part.neverRender = neverRender->as_bool().value();
				}

				if (auto cubes = bone.get("cubes")) {
					for (auto cube : cubes->as_array().value()) {
						auto& origin = cube.get("origin").value().as_array().value();
						auto& size = cube.get("size").value().as_array().value();

						const float posX = origin[0].as_f64().value();
						const float posY = origin[1].as_f64().value();
						const float posZ = origin[2].as_f64().value();

						const float sizeX = size[0].as_f64().value();
						const float sizeY = size[1].as_f64().value();
						const float sizeZ = size[2].as_f64().value();

						ModelPartBox modelBox;
						if (auto uv = cube.get("uv")) {
							modelBox.quads = std::visit(overload{
								[&] (json::Object& uv) {
									return createQuads_v1_12_0(posX, posY, posZ, sizeX, sizeY, sizeZ, uv);
								},
								[&] (json::Array& uv) {
									const float u = uv[0].as_f64().value();
									const float v = uv[1].as_f64().value();

									return createQuads_v1_8_0(posX, posY, posZ, sizeX, sizeY, sizeZ, u, v);
								},
								[&](auto&&) {
									return createQuads_v1_8_0(posX, posY, posZ, sizeX, sizeY, sizeZ, 0, 0);
								}
							}, uv->to_variant());
						} else {
							modelBox.quads = createQuads_v1_8_0(posX, posY, posZ, sizeX, sizeY, sizeZ, 0, 0);
						}

						part.cubes.emplace_back(std::move(modelBox));
					}
				}

				model.bones.emplace_back(std::move(part));
			}
		}
	}
}

void LoadGeometry(std::span<char> bytes) {
	using namespace std::string_view_literals;

	json::Parser parser(bytes);

	auto obj = parser.parse().value().as_object().value();

	auto& format_version = obj.extract("format_version").mapped().as_string().value();

	if (format_version == "1.8.0"sv) {
		LoadGeometry_v1_8_0(bytes, obj);
	} else if (format_version == "1.10.0"sv) {
		LoadGeometry_v1_8_0(bytes, obj);
	} else if (format_version == "1.12.0"sv) {
		LoadGeometry_v1_12_0(bytes, obj);
	} else {
		std::cout << "unsupported geometry format '" << format_version << "'" << std::endl;
	}
}

int main(int, char**) {
	ResourcePack resourcePack("assets/resource_packs/vanilla");
	resourcePack.LoadResources("models/entity", LoadGeometry);
	resourcePack.LoadResource("models/mobs.json", LoadGeometry);

	GameWindow window("Vulkan", 1280, 720);
	window.setMouseButtonCallback(&Mouse::handleMouseButton);
	window.setMousePositionCallback(&Mouse::handleMousePosition);
	window.setKeyboardCallback(&Keyboard::handleKeyInput);

	int width, height;
	window.getWindowSize(width, height);

	GameClient client{};
	client.init(&window);
	client.setRenderingSize(width, height);

	while (!window.shouldClose()) {
		if (client.wantToQuit()) {
			window.close();
			break;
		}

		client.tick();
	}
	return 0;
}