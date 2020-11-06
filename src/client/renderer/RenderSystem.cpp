#include "RenderSystem.hpp"

namespace {
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
}

void RenderSystem::init(GLFWwindow *window) {
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

	glfwCreateWindowSurface(_instance, window, nullptr, &_surface);

	_selectPhysicalDevice();

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
//			.enabledLayerCount = std::size(enabledLayers),
//			.ppEnabledLayerNames = std::data(enabledLayers),
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

void RenderSystem::terminate() {
	vmaDestroyAllocator(_allocator);

	_device.destroy(nullptr);

	_instance.destroySurfaceKHR(_surface, nullptr);
	_instance.destroy(nullptr);
}

bool RenderSystem::_selectPhysicalDevice() {
	auto physicalDevices = _instance.enumeratePhysicalDevices();

	for (auto physicalDevice : physicalDevices) {
		if (!_findQueueFamilies(physicalDevice, _surface)) {
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

bool RenderSystem::_findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
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

bool RenderSystem::_formatSupported(vk::Format format, vk::ImageTiling tiling, vk::FormatFeatureFlags flags) {
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

vk::Format RenderSystem::_findSupportedFormat(std::span<const vk::Format> formats, vk::ImageTiling tiling, vk::FormatFeatureFlags flags) {
	for (auto format : formats) {
		if (_formatSupported(format, tiling, flags)) {
			return format;
		}
	}

	return vk::Format::eUndefined;
}

vk::Format RenderSystem::getSupportedDepthFormat() {
	static constinit vk::Format formats[] { vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat };
	return _findSupportedFormat(formats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}
