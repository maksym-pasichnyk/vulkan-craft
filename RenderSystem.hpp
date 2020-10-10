#pragma once

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <iostream>

#include "mem_alloc.hpp"

#include "span.hpp"

struct RenderSystem {
	inline static constexpr vk::ApplicationInfo appInfo{
			.pApplicationName = nullptr,
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "Craft Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_2
	};
	inline static constexpr const char *enabledLayers[] = {
			"VK_LAYER_KHRONOS_validation"
	};

	inline static constexpr const char *device_extensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	inline static constexpr vk::PhysicalDeviceFeatures features {
			.fillModeNonSolid = VK_TRUE,
			.samplerAnisotropy = VK_TRUE
	};

	inline static RenderSystem* Get() {
		static constinit RenderSystem graphics;
		return &graphics;
	}

	inline static std::vector<const char *> getRequiredExtension() {
		uint32_t count = 0;
		auto extensions = glfwGetRequiredInstanceExtensions(&count);
		return std::vector<const char *>(extensions, extensions + count);
	}

	void initialize() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		_window = glfwCreateWindow(1280, 720, "Vulkan", NULL, NULL);

		glfwGetFramebufferSize(_window, &_width, &_height);

		auto extensions = getRequiredExtension();
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		vk::InstanceCreateInfo instanceCreateInfo {
				.pApplicationInfo = &appInfo,
				.enabledLayerCount = std::size(enabledLayers),
				.ppEnabledLayerNames = std::data(enabledLayers),
				.enabledExtensionCount = uint32_t(extensions.size()),
				.ppEnabledExtensionNames = extensions.data(),
		};
		_instance = vk::createInstance(instanceCreateInfo);

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT{
				.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
						vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
						vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
				.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
						vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
						vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
				.pfnUserCallback = DebugCallback,
		};

		createDebugUtilsMessengerEXT(_instance, &debugCreateInfo, nullptr, &debugUtilsMessenger);

		glfwCreateWindowSurface(_instance, _window, nullptr, &_surface);

		selectPhysicalDevice();

		const float queuePriority = 1.0f;

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};
		queueCreateInfos.reserve(2);

		vk::DeviceQueueCreateInfo graphicsQueueCreateInfo {
			.queueFamilyIndex = _graphicsFamily,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};
		queueCreateInfos.emplace_back(graphicsQueueCreateInfo);

		if (_graphicsFamily != _presentFamily) {
			vk::DeviceQueueCreateInfo presentQueueCreateInfo {
				.queueFamilyIndex = _presentFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			};

			queueCreateInfos.emplace_back(presentQueueCreateInfo);
		}

		vk::DeviceCreateInfo deviceCreateInfo {
			.queueCreateInfoCount = uint32_t(std::size(queueCreateInfos)),
			.pQueueCreateInfos = std::data(queueCreateInfos),
			.enabledLayerCount = std::size(enabledLayers),
			.ppEnabledLayerNames = std::data(enabledLayers),
			.enabledExtensionCount = std::size(device_extensions),
			.ppEnabledExtensionNames = std::data(device_extensions),
			.pEnabledFeatures = &features
		};

		_device = _physicalDevice.createDevice(deviceCreateInfo, nullptr);

		_presentQueue = _device.getQueue(_presentFamily, 0);
		_graphicsQueue = _device.getQueue(_graphicsFamily, 0);

		VmaAllocatorCreateInfo allocatorCreateInfo{
			.physicalDevice = _physicalDevice,
			.device = _device,
			.instance = _instance,
			.vulkanApiVersion = VK_API_VERSION_1_2
		};

		vmaCreateAllocator(&allocatorCreateInfo, &_allocator);
	}

	void terminate() {
		vmaDestroyAllocator(_allocator);

		_device.destroy();
		_instance.destroy();

		glfwDestroyWindow(_window);
		glfwTerminate();
	}

private:
	inline static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cout << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	inline static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	inline bool selectPhysicalDevice() {
		auto physicalDevices = _instance.enumeratePhysicalDevices();

		for (auto physicalDevice : physicalDevices) {
			if (!FindQueueFamilies(physicalDevice, _surface)) {
				continue;
			}

			uint32_t surface_format_count = 0;
			physicalDevice.getSurfaceFormatsKHR(_surface, &surface_format_count, nullptr);
			if (surface_format_count == 0) {
				continue;
			}

			uint32_t present_mode_count = 0;
			physicalDevice.getSurfacePresentModesKHR(_surface, &present_mode_count, nullptr);
			if (present_mode_count == 0) {
				continue;
			}

			_physicalDevice = physicalDevice;
			return true;
		}

		return false;
	}

	inline bool FindQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
		const auto properties = device.getQueueFamilyProperties();

		uint32_t graphics_family = -1;
		uint32_t present_family = -1;
		for (uint32_t i = 0; i < uint32_t(properties.size()); i++) {
			if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
				graphics_family = i;
			}

			if (device.getSurfaceSupportKHR(i, surface)) {
				present_family = i;
			}

			if ((graphics_family != -1) && (present_family != -1)) {
				_graphicsFamily = graphics_family;
				_presentFamily = present_family;
				return true;
			}
		}
		return false;
	}

	inline bool isFormatSupported(vk::Format format, vk::ImageTiling tiling, vk::FormatFeatureFlags flags) {
		vk::FormatProperties formatProperties = _physicalDevice.getFormatProperties(format);

		switch (tiling) {
		case vk::ImageTiling::eLinear:
			return (formatProperties.linearTilingFeatures & flags) == flags;
		case vk::ImageTiling::eOptimal:
			return (formatProperties.optimalTilingFeatures & flags) == flags;
		default:
			return false;
		}
	}

	inline vk::Format findSupportedFormat(span<const vk::Format> formats, vk::ImageTiling tiling, vk::FormatFeatureFlags flags) {
		for (auto format : formats) {
			if (isFormatSupported(format, tiling, flags)) {
				return format;
			}
		}

		return vk::Format::eUndefined;
	}

public:
	inline vk::Format getSupportedDepthFormat() {
		static constinit vk::Format formats[] { vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat };
		return findSupportedFormat(formats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}

public:
	int width() {
		return _width;
	}

	int height() {
		return _height;
	}

	GLFWwindow* window() {
		return _window;
	}

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
	GLFWwindow* _window{nullptr};
	int _width{0}, _height{0};

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