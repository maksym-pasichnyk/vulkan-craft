#pragma once

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <iostream>

#include "mem_alloc.hpp"

#include <span>

struct Allocator {

};

struct RenderSystem {
	inline static constexpr vk::ApplicationInfo appInfo{
			.pApplicationName = nullptr,
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "Craft Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_2
	};
	inline static constexpr const char *enabledLayers[1] = {
			"VK_LAYER_KHRONOS_validation"
	};

	inline static constexpr const char *device_extensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	inline static constexpr vk::PhysicalDeviceFeatures features {
			.fillModeNonSolid = VK_TRUE,
			.samplerAnisotropy = VK_TRUE
	};

	inline static RenderSystem* Instance() {
		static /*constinit*/ RenderSystem graphics;
		return &graphics;
	}

	inline static std::vector<const char *> getRequiredExtension() {
		uint32_t count = 0;
		auto extensions = glfwGetRequiredInstanceExtensions(&count);
		return std::vector<const char *>(extensions, extensions + count);
	}

	void init(GLFWwindow* window);

	void terminate();

private:
	bool _selectPhysicalDevice();

	bool _findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);

	bool _formatSupported(vk::Format format, vk::ImageTiling tiling, vk::FormatFeatureFlags flags);

	vk::Format _findSupportedFormat(std::span<const vk::Format> formats, vk::ImageTiling tiling, vk::FormatFeatureFlags flags);

public:
	vk::Format getSupportedDepthFormat();

public:
	vk::Instance instance() {
		return _instance;
	}

	vk::PhysicalDevice physicalDevice() {
		return _physicalDevice;
	}

	vk::Device device() {
		return _device;
	}

	VmaAllocator allocator() {
		return _allocator;
	}

	vk::SurfaceKHR surface() {
		return _surface;
	}

	uint32_t graphicsFamily() {
		return _graphicsFamily;
	}

	uint32_t presentFamily() {
		return _presentFamily;
	}

	vk::Queue graphicsQueue() {
		return _graphicsQueue;
	}

	vk::Queue presentQueue() {
		return _presentQueue;
	}

private:
	vk::Instance _instance;
	vk::PhysicalDevice _physicalDevice;
	vk::Device _device;

	VmaAllocator _allocator{nullptr};

	VkDebugUtilsMessengerEXT debugUtilsMessenger{nullptr};
	VkSurfaceKHR _surface{nullptr};

	uint32_t _graphicsFamily{0};
	uint32_t _presentFamily{0};

	vk::Queue _presentQueue;
	vk::Queue _graphicsQueue;
};