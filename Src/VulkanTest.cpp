#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw3native.h>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <iostream>
#include "Calculations/General/Types.hpp"
#include "vulkan/vulkan.h"
#include <optional>
#include <set>
#include <algorithm>
#include <limits>
#include <fstream>
#include <vulkan/vulkan_raii.hpp>
#include "VulkanWrappers/Providers/Instance.hpp"
class Application {
private:
#ifdef NDEBUG
	const b8 enable_validation_layers = false;
#else
	const b8 enable_validation_layers = true;
#endif
	GLFWwindow* window_ptr = {};

	VkDebugUtilsMessengerEXT debug_messenger = {};
	const std::vector<const char*> validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	Instance instance = {};

	VkPhysicalDevice physical_device = {};
	std::vector<const char*> required_device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	struct QueueFamilyIndices {
		std::optional<u32> graphics_family = {};
		std::optional<u32> present_family = {};
	};
	VkDevice logical_device = {};
	VkQueue graphics_queue = {};
	VkQueue present_queue = {};

	VkSurfaceKHR window_surface = {};
	
	struct SwapChainDetails {
		VkSurfaceCapabilitiesKHR capabilities = {};
		std::vector<VkSurfaceFormatKHR> formats = {};
		std::vector<VkPresentModeKHR> present_modes = {};
	};
	VkSwapchainKHR swap_chain = {};
	std::vector<VkImage> swap_chain_images = {};
	VkFormat chosen_format = {};
	VkExtent2D chosen_extent = {};

	std::vector<VkImageView> swap_chain_image_views = {};

	VkPipelineLayout pipeline_layout = {};
	VkPipeline graphics_pipeline = {};
	
	VkCommandPool command_pool = {};
	VkCommandBuffer command_buffer = {};
	
	VkSemaphore image_available_semaphore = {};
	std::vector<VkSemaphore> rendering_finished_semaphores = {};
	VkFence in_flight_fence = {};

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window_ptr = glfwCreateWindow(1000, 1000, "hello vulkan", nullptr, nullptr);
	}

	b8 checkValidationLayerSupport() {
		u32 layercount;
		vkEnumerateInstanceLayerProperties(&layercount, nullptr);

		std::vector<VkLayerProperties> availablevalidationlayers(layercount);
		vkEnumerateInstanceLayerProperties(&layercount, availablevalidationlayers.data());

		for (const char* layername : validation_layers) {
			b8 layerfound = false;
			for (const VkLayerProperties& avalablelayerproperties : availablevalidationlayers) {
				if (strcmp(avalablelayerproperties.layerName, layername) == 0) {
					layerfound = true;
				}
			}
			if (!layerfound) {
				return false;
			}
		}
		return true;
	}

	std::vector<std::string> getRequiredExtensions() {
		u32 glfwextensioncount = {};
		const char** glfwextensions = {};
		glfwextensions = glfwGetRequiredInstanceExtensions(&glfwextensioncount);

		std::vector<std::string> extensions(glfwextensions, glfwextensions + glfwextensioncount);

		if (enable_validation_layers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageseverity,
		VkDebugUtilsMessageTypeFlagsEXT messagetype,
		const VkDebugUtilsMessengerCallbackDataEXT* pcallbackdata,
		void* userdata
	) {
		std::cerr << "validation layer: " << pcallbackdata->pMessage << std::endl;
		std::cerr << std::endl;
		return VK_FALSE;
	}

	VkResult createDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pdebugcreationinfo,
		const VkAllocationCallbacks* pallocator,
		VkDebugUtilsMessengerEXT* pdebugmessenger
	) {
		PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func) {
			return func(instance, pdebugcreationinfo, pallocator, pdebugmessenger);
		}
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugmessengercreationinfo) {
		debugmessengercreationinfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugmessengercreationinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugmessengercreationinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugmessengercreationinfo.pfnUserCallback = debugCallback;
		debugmessengercreationinfo.pUserData = nullptr;
	}

	void setUpDebugMessenger() {
		VkDebugUtilsMessengerCreateInfoEXT debugmessengercreationinfo = {};
		populateDebugMessengerCreateInfo(debugmessengercreationinfo);
		if (createDebugUtilsMessengerEXT(instance.get(), &debugmessengercreationinfo, nullptr, &debug_messenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to setup debug messenger");
		}
	}

	void createInstance() {
		if (enable_validation_layers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested but not available");
		}

		instance.init({ 1,4,0 }, { 0,0,1 }, { 0,0,1 }, "vulkan lol", "VulkanOGE", 0, getRequiredExtensions(), nullptr, nullptr);
	}

	QueueFamilyIndices findAllAvailableQueueFamilies(VkPhysicalDevice physicaldevice) {
		QueueFamilyIndices queuefamilyindices;
		u32 queuefamilycount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicaldevice, &queuefamilycount, nullptr);

		std::vector<VkQueueFamilyProperties> queuefamilyproperties(queuefamilycount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicaldevice, &queuefamilycount, queuefamilyproperties.data());
		i32 i = 0;
		for (const auto& queuefamily : queuefamilyproperties) {
			VkBool32 presentsupported = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicaldevice, i, window_surface, &presentsupported);
			if (queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				queuefamilyindices.graphics_family = i;
			}
			if (presentsupported) {
				queuefamilyindices.present_family = i;
			}
			i++;
		}
		return queuefamilyindices;
	}

	b8 requiredQueueFamiliesExist(QueueFamilyIndices indices) {
		return indices.graphics_family.has_value() && indices.present_family.has_value();
	}
	
	b8 requiredExtensionsSupported(VkPhysicalDevice physicaldevice) { // another way to do it is checkValidationLayerSupport() algorithm
		u32 extensioncount = 0;
		vkEnumerateDeviceExtensionProperties(physicaldevice, nullptr, &extensioncount, nullptr);

		std::vector<VkExtensionProperties> availableextensionproperties(extensioncount);
		vkEnumerateDeviceExtensionProperties(physicaldevice, nullptr, &extensioncount, availableextensionproperties.data());

		std::set<std::string> uniquerequiredextensions(required_device_extensions.begin(), required_device_extensions.end());

		for (const auto& availableextension : availableextensionproperties) {
			uniquerequiredextensions.erase(availableextension.extensionName);
		}
		return uniquerequiredextensions.empty();
	}

	SwapChainDetails getSwapChainDetails(VkPhysicalDevice physicaldevice) {
		SwapChainDetails swapchaindetails = {};

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicaldevice, window_surface, &swapchaindetails.capabilities);

		u32 formatcount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicaldevice, window_surface, &formatcount, nullptr);
		if (formatcount != 0) {
			swapchaindetails.formats.resize(formatcount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicaldevice, window_surface, &formatcount, swapchaindetails.formats.data());
		}

		u32 presentmodecount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicaldevice, window_surface, &presentmodecount, nullptr);
		if (presentmodecount != 0) {
			swapchaindetails.present_modes.resize(presentmodecount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicaldevice, window_surface, &presentmodecount, swapchaindetails.present_modes.data());
		}
		return swapchaindetails;
	}
	
	b8 swapChainSuitable(VkPhysicalDevice physicaldevice) {
		SwapChainDetails swapchaindetails = getSwapChainDetails(physicaldevice);
		return !swapchaindetails.formats.empty() && !swapchaindetails.present_modes.empty();
	}

	b8 isPhysicalDeviceSuitable(VkPhysicalDevice physicaldevice) {
		QueueFamilyIndices indices = findAllAvailableQueueFamilies(physicaldevice);
		return requiredQueueFamiliesExist(indices) && requiredExtensionsSupported(physicaldevice) && swapChainSuitable(physicaldevice);
	}

	void pickPhysicalDevice() {
		physical_device = VK_NULL_HANDLE;
		u32 devicecount = 0;
		vkEnumeratePhysicalDevices(instance.get(), &devicecount, nullptr);

		if (devicecount == 0) {
			throw std::runtime_error("failed to find a gpu");
		}

		std::vector<VkPhysicalDevice> physicaldevices(devicecount);
		vkEnumeratePhysicalDevices(instance.get(), &devicecount, physicaldevices.data());

		for (const auto& device : physicaldevices) {
			if (isPhysicalDeviceSuitable(device)) {
				physical_device = device;
				break;
			}
		}

		if (physical_device == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a gpu that supports Vulkan");
		}
	}

	void createLogicalDevice() {
		QueueFamilyIndices queuefamilyindices = findAllAvailableQueueFamilies(physical_device);
		std::set<u32> uniquequeuefamilyindices = { queuefamilyindices.graphics_family.value(), queuefamilyindices.present_family.value() };
		std::vector<VkDeviceQueueCreateInfo> queuecreationinfos = {};
		f32 queuepriority = 1.0f;

		for (const auto& queuefamilyindex : uniquequeuefamilyindices) {
			VkDeviceQueueCreateInfo queuecreationinfo = {};
			queuecreationinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queuecreationinfo.queueFamilyIndex = queuefamilyindex;
			queuecreationinfo.queueCount = 1;
			queuecreationinfo.pQueuePriorities = &queuepriority;
			queuecreationinfos.push_back(queuecreationinfo);
		}

		VkPhysicalDeviceVulkan13Features vulkan13requiredphysicaldevicefeatures = {};
		vulkan13requiredphysicaldevicefeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		vulkan13requiredphysicaldevicefeatures.dynamicRendering = VK_TRUE;

		VkPhysicalDeviceFeatures2 requiredphysicaldevicefeatures = {};
		requiredphysicaldevicefeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		requiredphysicaldevicefeatures.pNext = &vulkan13requiredphysicaldevicefeatures;

		VkDeviceCreateInfo devicecreationinfo = {};
		devicecreationinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		devicecreationinfo.pQueueCreateInfos = queuecreationinfos.data();
		devicecreationinfo.queueCreateInfoCount = static_cast<u32>(queuecreationinfos.size());
		devicecreationinfo.pEnabledFeatures = nullptr;
		devicecreationinfo.enabledExtensionCount = static_cast<u32>(required_device_extensions.size());
		devicecreationinfo.ppEnabledExtensionNames = required_device_extensions.data();
		devicecreationinfo.enabledLayerCount = 0;
		devicecreationinfo.pNext = &requiredphysicaldevicefeatures;
		
		if (vkCreateDevice(physical_device, &devicecreationinfo, nullptr, &logical_device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device");
		}
		vkGetDeviceQueue(logical_device, queuefamilyindices.graphics_family.value(), 0, &graphics_queue);
		vkGetDeviceQueue(logical_device, queuefamilyindices.present_family.value(), 0, &present_queue);
	}

	void createWindowSurface() {
		if (glfwCreateWindowSurface(instance.get(), window_ptr, nullptr, &window_surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	VkSurfaceFormatKHR pickSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
		for (const auto& format : formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
				return format;
			}
		}
		return formats[0];
	}

	VkPresentModeKHR pickSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentmodes) {
		for (const auto& presentmode : presentmodes) {
			if (presentmode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return presentmode;
			}
		}
		return	VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D pickSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != UINT32_MAX && capabilities.currentExtent.height != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			i32 width = 0;
			i32 height = 0;
			glfwGetFramebufferSize(window_ptr, &width, &height);
			VkExtent2D actualextent = {};
			actualextent.height = static_cast<u32>(height);
			actualextent.width = static_cast<u32>(width);
			actualextent.width = std::clamp(actualextent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualextent.height = std::clamp(actualextent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			return actualextent;
		}
	}

	void createSwapChain() {
		SwapChainDetails swapchaindetails = getSwapChainDetails(physical_device);
		VkSurfaceFormatKHR format = pickSwapChainFormat(swapchaindetails.formats);
		VkPresentModeKHR presentmode = pickSwapChainPresentMode(swapchaindetails.present_modes);
		VkExtent2D extent = pickSwapChainExtent(swapchaindetails.capabilities);

		u32 imagecount = swapchaindetails.capabilities.minImageCount + 1;
		if (imagecount > swapchaindetails.capabilities.maxImageCount && swapchaindetails.capabilities.maxImageCount > 0) {
			imagecount = swapchaindetails.capabilities.maxImageCount;
		}

		QueueFamilyIndices queuefamilyindices = findAllAvailableQueueFamilies(physical_device);
		u32 queuefamilyindicesarr[] = { queuefamilyindices.graphics_family.value(), queuefamilyindices.present_family.value() };

		VkSwapchainCreateInfoKHR swapchaincreationinfo = {};
		swapchaincreationinfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchaincreationinfo.surface = window_surface;
		swapchaincreationinfo.minImageCount = imagecount;
		swapchaincreationinfo.imageFormat = format.format;
		swapchaincreationinfo.imageColorSpace = format.colorSpace;
		swapchaincreationinfo.presentMode = presentmode;
		swapchaincreationinfo.imageExtent = extent;
		swapchaincreationinfo.imageArrayLayers = 1;
		swapchaincreationinfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchaincreationinfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchaincreationinfo.clipped = VK_TRUE;
		swapchaincreationinfo.oldSwapchain = VK_NULL_HANDLE;
		swapchaincreationinfo.preTransform = swapchaindetails.capabilities.currentTransform;
		if (queuefamilyindices.graphics_family != queuefamilyindices.present_family) {
			swapchaincreationinfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchaincreationinfo.queueFamilyIndexCount = 2;
			swapchaincreationinfo.pQueueFamilyIndices = queuefamilyindicesarr;
		}
		else {
			swapchaincreationinfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchaincreationinfo.queueFamilyIndexCount = 0;
			swapchaincreationinfo.pQueueFamilyIndices = nullptr;
		}

		if (vkCreateSwapchainKHR(logical_device, &swapchaincreationinfo, nullptr, &swap_chain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a swap chain");
		}

		u32 swapchainimagecount = 0;
		vkGetSwapchainImagesKHR(logical_device, swap_chain, &swapchainimagecount, nullptr);

		swap_chain_images.resize(swapchainimagecount);
		vkGetSwapchainImagesKHR(logical_device, swap_chain, &swapchainimagecount, swap_chain_images.data());

		rendering_finished_semaphores.resize(swapchainimagecount);

		chosen_extent = extent;
		chosen_format = format.format;
	}

	void createImageViews() {
		swap_chain_image_views.resize(swap_chain_images.size());
		
		for (usize i = 0; i < swap_chain_images.size(); i++) {
			VkImageViewCreateInfo imageviewcreationinfo = {};
			imageviewcreationinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageviewcreationinfo.image = swap_chain_images[i];
			imageviewcreationinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageviewcreationinfo.format = chosen_format;
			imageviewcreationinfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageviewcreationinfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageviewcreationinfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageviewcreationinfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageviewcreationinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageviewcreationinfo.subresourceRange.baseArrayLayer = 0;
			imageviewcreationinfo.subresourceRange.baseMipLevel = 0;
			imageviewcreationinfo.subresourceRange.layerCount = 1;
			imageviewcreationinfo.subresourceRange.levelCount = 1;

			if (vkCreateImageView(logical_device, &imageviewcreationinfo, nullptr, &swap_chain_image_views[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create image views");
			}
		}
	}

	std::vector<c8> readFile(const std::string& filename) {
		std::ifstream input(filename, std::ios::ate | std::ios::binary);
		if (!input.is_open()) {
			throw std::runtime_error("failed to open input file");
		}

		usize filesize = static_cast<usize>(input.tellg());
		std::vector<c8> charbuffer(filesize);

		input.seekg(0);
		input.read(charbuffer.data(), filesize);
		input.close();

		return charbuffer;
	}

	VkShaderModule createShaderModule(const std::vector<c8>& shadercode) {
		VkShaderModuleCreateInfo shadermodulecreationinfo = {};
		shadermodulecreationinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;;
		shadermodulecreationinfo.codeSize = shadercode.size();
		shadermodulecreationinfo.pCode = reinterpret_cast<const u32*>(shadercode.data());
		
		VkShaderModule shadermodule = {};
		if (vkCreateShaderModule(logical_device, &shadermodulecreationinfo, nullptr, &shadermodule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}
		return shadermodule;
	}

	void createGraphicsPipeline() {
		std::vector<c8> vertexshadercode = readFile("Src\\CompiledShaders\\VertexShader.spv");
		std::vector<c8> fragmentshadercode = readFile("Src\\CompiledShaders\\FragmentShader.spv");

		VkShaderModule vertexshadermodule = createShaderModule(vertexshadercode);
		VkShaderModule fragmentshadermodule = createShaderModule(fragmentshadercode);

		VkPipelineShaderStageCreateInfo vertexshaderpipelineshaderstagecreationinfo = {};
		vertexshaderpipelineshaderstagecreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexshaderpipelineshaderstagecreationinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexshaderpipelineshaderstagecreationinfo.pName = "main";
		vertexshaderpipelineshaderstagecreationinfo.module = vertexshadermodule;
		vertexshaderpipelineshaderstagecreationinfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragmentshaderpipelineshaderstagecreationinfo = {};
		fragmentshaderpipelineshaderstagecreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentshaderpipelineshaderstagecreationinfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentshaderpipelineshaderstagecreationinfo.pName = "main";
		fragmentshaderpipelineshaderstagecreationinfo.module = fragmentshadermodule;
		fragmentshaderpipelineshaderstagecreationinfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderstages[] = { vertexshaderpipelineshaderstagecreationinfo, fragmentshaderpipelineshaderstagecreationinfo };

		std::vector<VkDynamicState> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicstatecreationinfo = {};
		dynamicstatecreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicstatecreationinfo.dynamicStateCount = static_cast<u32>(dynamic_states.size());
		dynamicstatecreationinfo.pDynamicStates = dynamic_states.data();

		VkPipelineViewportStateCreateInfo viewportcreationinfo = {};
		viewportcreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportcreationinfo.viewportCount = 1;
		viewportcreationinfo.pViewports = nullptr;
		viewportcreationinfo.scissorCount = 1;
		viewportcreationinfo.pScissors = nullptr;

		VkPipelineVertexInputStateCreateInfo vertexinputstatecreationinfo = {};
		vertexinputstatecreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexinputstatecreationinfo.vertexAttributeDescriptionCount = 0;
		vertexinputstatecreationinfo.pVertexAttributeDescriptions = nullptr;
		vertexinputstatecreationinfo.vertexBindingDescriptionCount = 0;
		vertexinputstatecreationinfo.pVertexBindingDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputassemblycreationinfo = {};
		inputassemblycreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputassemblycreationinfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputassemblycreationinfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizationstatecreationinfo = {};
		rasterizationstatecreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationstatecreationinfo.depthClampEnable = VK_FALSE;
		rasterizationstatecreationinfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationstatecreationinfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationstatecreationinfo.lineWidth = 1.0f;
		rasterizationstatecreationinfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationstatecreationinfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationstatecreationinfo.depthBiasEnable = VK_FALSE;
		rasterizationstatecreationinfo.depthBiasConstantFactor = 0.0f;
		rasterizationstatecreationinfo.depthBiasSlopeFactor = 0.0f;
		rasterizationstatecreationinfo.depthBiasClamp = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisamplestatecreationinfo = {};
		multisamplestatecreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplestatecreationinfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplestatecreationinfo.sampleShadingEnable = VK_FALSE;
		multisamplestatecreationinfo.alphaToCoverageEnable = VK_FALSE;
		multisamplestatecreationinfo.alphaToOneEnable = VK_FALSE;
		multisamplestatecreationinfo.pSampleMask = nullptr;
		multisamplestatecreationinfo.minSampleShading = 1.0f;

		VkPipelineColorBlendAttachmentState colorblendattachmentstate = {};
		colorblendattachmentstate.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorblendattachmentstate.blendEnable = VK_FALSE;
		colorblendattachmentstate.alphaBlendOp = VK_BLEND_OP_ADD;
		colorblendattachmentstate.colorBlendOp = VK_BLEND_OP_ADD;
		colorblendattachmentstate.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorblendattachmentstate.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorblendattachmentstate.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorblendattachmentstate.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

		VkPipelineColorBlendStateCreateInfo colorblendstatecreationinfo = {};
		colorblendstatecreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorblendstatecreationinfo.logicOpEnable = VK_FALSE;
		colorblendstatecreationinfo.logicOp = VK_LOGIC_OP_COPY;
		colorblendstatecreationinfo.attachmentCount = 1;
		colorblendstatecreationinfo.pAttachments = &colorblendattachmentstate;
		colorblendstatecreationinfo.blendConstants[0] = 0.0f;
		colorblendstatecreationinfo.blendConstants[1] = 0.0f;
		colorblendstatecreationinfo.blendConstants[2] = 0.0f;
		colorblendstatecreationinfo.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelinelayoutcreationinfo = {};
		pipelinelayoutcreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelinelayoutcreationinfo.setLayoutCount = 0;
		pipelinelayoutcreationinfo.pSetLayouts = nullptr;
		pipelinelayoutcreationinfo.pushConstantRangeCount = 0;
		pipelinelayoutcreationinfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(logical_device, &pipelinelayoutcreationinfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}

		VkPipelineRenderingCreateInfo pipelinerenderingcreationinfo = {};
		pipelinerenderingcreationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelinerenderingcreationinfo.colorAttachmentCount = 1;
		pipelinerenderingcreationinfo.pColorAttachmentFormats = &chosen_format;

		VkGraphicsPipelineCreateInfo graphicspipelinecreationinfo = {};
		graphicspipelinecreationinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicspipelinecreationinfo.stageCount = 2;
		graphicspipelinecreationinfo.pStages = shaderstages;
		graphicspipelinecreationinfo.pColorBlendState = &colorblendstatecreationinfo;
		graphicspipelinecreationinfo.pInputAssemblyState = &inputassemblycreationinfo;
		graphicspipelinecreationinfo.pVertexInputState = &vertexinputstatecreationinfo;
		graphicspipelinecreationinfo.pDynamicState = &dynamicstatecreationinfo;
		graphicspipelinecreationinfo.pMultisampleState = &multisamplestatecreationinfo;
		graphicspipelinecreationinfo.pViewportState = &viewportcreationinfo;
		graphicspipelinecreationinfo.pRasterizationState = &rasterizationstatecreationinfo;
		graphicspipelinecreationinfo.layout = pipeline_layout;
		graphicspipelinecreationinfo.pDepthStencilState = nullptr;
		graphicspipelinecreationinfo.renderPass = nullptr;
		graphicspipelinecreationinfo.pNext = &pipelinerenderingcreationinfo;
		graphicspipelinecreationinfo.subpass = 0;
		graphicspipelinecreationinfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicspipelinecreationinfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &graphicspipelinecreationinfo, nullptr, &graphics_pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline");
		}

		vkDestroyShaderModule(logical_device, vertexshadermodule, nullptr);
		vkDestroyShaderModule(logical_device, fragmentshadermodule, nullptr);
	}

	void createCommandPool() {
		QueueFamilyIndices queuefamilyindices = findAllAvailableQueueFamilies(physical_device);

		VkCommandPoolCreateInfo commandpoolcreationinfo = {};
		commandpoolcreationinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandpoolcreationinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(logical_device, &commandpoolcreationinfo, nullptr, &command_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool");
		}


	}

	void createCommandBuffer() {
		VkCommandBufferAllocateInfo allocationinfo = {};
		allocationinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocationinfo.commandPool = command_pool;
		allocationinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocationinfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(logical_device, &allocationinfo, &command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer");
		}
	}

	void recordCommandBuffer(VkCommandBuffer commandbuffer, u32 imageindex) {
		VkCommandBufferBeginInfo commandbufferbegininfo = {};
		commandbufferbegininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandbufferbegininfo.pInheritanceInfo = nullptr;
		commandbufferbegininfo.flags = 0;

		if (vkBeginCommandBuffer(commandbuffer, &commandbufferbegininfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording to runtime buffer");
		}

		VkImageSubresourceRange imagesubresourcerange = {};
		imagesubresourcerange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imagesubresourcerange.baseArrayLayer = 0;
		imagesubresourcerange.layerCount = 1;
		imagesubresourcerange.baseMipLevel = 0;
		imagesubresourcerange.levelCount = 1;

		VkImageMemoryBarrier colorattachmentmemorybarrier = {};
		colorattachmentmemorybarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		colorattachmentmemorybarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorattachmentmemorybarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorattachmentmemorybarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		colorattachmentmemorybarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		colorattachmentmemorybarrier.image = swap_chain_images[imageindex];
		colorattachmentmemorybarrier.subresourceRange = imagesubresourcerange;
		colorattachmentmemorybarrier.srcAccessMask = 0;
		colorattachmentmemorybarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		vkCmdPipelineBarrier(
			commandbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &colorattachmentmemorybarrier
		);

		VkRenderingAttachmentInfo colorattachmentinfo = {};
		colorattachmentinfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorattachmentinfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorattachmentinfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorattachmentinfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorattachmentinfo.imageView = swap_chain_image_views[imageindex];
		colorattachmentinfo.clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

		VkRenderingInfo renderinginfo = {};
		renderinginfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderinginfo.colorAttachmentCount = 1;
		renderinginfo.pColorAttachments = &colorattachmentinfo;
		renderinginfo.renderArea = { {0,0}, chosen_extent };
		renderinginfo.layerCount = 1;

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<f32>(chosen_extent.width);
		viewport.height = static_cast<f32>(chosen_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent = chosen_extent;
		scissor.offset = { 0, 0 };

		vkCmdBeginRendering(commandbuffer, &renderinginfo);
		vkCmdSetViewport(commandbuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandbuffer, 0, 1, &scissor);
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
		vkCmdDraw(commandbuffer, 3, 1, 0, 0);
		vkCmdEndRendering(commandbuffer);

		VkImageMemoryBarrier presentmemorybarrier = {};
		presentmemorybarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		presentmemorybarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		presentmemorybarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		presentmemorybarrier.image = swap_chain_images[imageindex];
		presentmemorybarrier.subresourceRange = imagesubresourcerange;
		presentmemorybarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		presentmemorybarrier.dstAccessMask = 0;
		presentmemorybarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		presentmemorybarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vkCmdPipelineBarrier(
			commandbuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, 0,
			1, &presentmemorybarrier
		);

		if (vkEndCommandBuffer(commandbuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to end command buffer");
		}
	}

	void createSynchronizationObjects() {
		VkSemaphoreCreateInfo semaphorecreationinfo = {};
		semaphorecreationinfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fencecreationinfo = {};
		fencecreationinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fencecreationinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (usize i = 0; i < rendering_finished_semaphores.size(); i++) {
			if (vkCreateSemaphore(logical_device, &semaphorecreationinfo, nullptr, &rendering_finished_semaphores[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create semaphores");
			}
		}
		if (vkCreateSemaphore(logical_device, &semaphorecreationinfo, nullptr, &image_available_semaphore) != VK_SUCCESS) {
			throw std::runtime_error("failed to create semaphores");
		}

		if (vkCreateFence(logical_device, &fencecreationinfo, nullptr, &in_flight_fence) != VK_SUCCESS) {
			throw std::runtime_error("failed to create fences");
		}
	}

	void initVulkan() {
		createInstance();
		if (enable_validation_layers) {
			setUpDebugMessenger();
		}
		createWindowSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createGraphicsPipeline();
		createCommandPool();
		createCommandBuffer();
		createSynchronizationObjects();
	}

	void drawFrame() {
		vkWaitForFences(logical_device, 1, &in_flight_fence, VK_TRUE, UINT64_MAX);
		vkResetFences(logical_device, 1, &in_flight_fence);

		u32 imageindex = 0;
		vkAcquireNextImageKHR(logical_device, swap_chain, UINT64_MAX, image_available_semaphore, VK_NULL_HANDLE, &imageindex);

		vkResetCommandBuffer(command_buffer, 0);
		recordCommandBuffer(command_buffer, imageindex);

		VkSemaphore imagesemaphores[] = { image_available_semaphore };

		VkSemaphore signalsemaphores[] = { rendering_finished_semaphores[imageindex] };

		VkPipelineStageFlags waitstages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submissioninfo = {};
		submissioninfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submissioninfo.waitSemaphoreCount = 1;
		submissioninfo.pWaitSemaphores = imagesemaphores;
		submissioninfo.pWaitDstStageMask = waitstages;
		submissioninfo.commandBufferCount = 1;
		submissioninfo.pCommandBuffers = &command_buffer;
		submissioninfo.signalSemaphoreCount = 1;
		submissioninfo.pSignalSemaphores = signalsemaphores;

		if (vkQueueSubmit(graphics_queue, 1, &submissioninfo, in_flight_fence) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit to graphics queue");
		}

		VkSwapchainKHR swapchains[] = { swap_chain };

		VkPresentInfoKHR presentationinfo = {};
		presentationinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentationinfo.waitSemaphoreCount = 1;
		presentationinfo.pWaitSemaphores = signalsemaphores;
		presentationinfo.swapchainCount = 1;
		presentationinfo.pSwapchains = swapchains;
		presentationinfo.pImageIndices = &imageindex;
		presentationinfo.pResults = nullptr;

		vkQueuePresentKHR(present_queue, &presentationinfo);
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window_ptr)) {
			glfwPollEvents();
			drawFrame();
		}
	}
	void destroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugmessenger,
		const VkAllocationCallbacks* pallocator
	) {
		PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func) {
			func(instance, debugmessenger, pallocator);
		}
	}

	void destroySwapChainImageViews() {
		for (auto& imageview : swap_chain_image_views) {
			vkDestroyImageView(logical_device, imageview, nullptr);
		}
	}

	void destroySynchronizationObjects() {
		vkDestroySemaphore(logical_device, image_available_semaphore, nullptr);
		for (usize i = 0; i < rendering_finished_semaphores.size(); i++) {
			vkDestroySemaphore(logical_device, rendering_finished_semaphores[i], nullptr);
		}
		vkDestroyFence(logical_device, in_flight_fence, nullptr);
	}

	void cleanUp() {
		vkDeviceWaitIdle(logical_device);
		if (enable_validation_layers) {
			destroyDebugUtilsMessengerEXT(instance.get(), debug_messenger, nullptr);
		}
		destroySynchronizationObjects();
		vkDestroyCommandPool(logical_device, command_pool, nullptr);
		vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
		vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
		destroySwapChainImageViews();
		vkDestroySwapchainKHR(logical_device, swap_chain, nullptr);
		vkDestroyDevice(logical_device, nullptr);
		vkDestroySurfaceKHR(instance.get(), window_surface, nullptr);
		glfwDestroyWindow(window_ptr);
		glfwTerminate();
	}

public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}
};
int main() {
	Application app;
	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

}