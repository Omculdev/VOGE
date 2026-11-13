#include "VulkanWrappers/Providers/DebugMessenger.hpp"
#include "Utils/Macros/DebugMacros.hpp"
#include "vulkan/vulkan.h"



[[nodiscard]] b8 DebugMessenger::basicInit(
	const std::string& caller_sig,
	const Instance& instance, 
	const VkDebugUtilsMessengerCreateInfoEXT & debug_messenger_creation_info
) {
	findDebugMessengerCreationFunction(instance);
	findDebugMessengerDestuctionFunction(instance);

	FATAL_ASSERT(
		caller_sig,
		debug_messenger_creation_function_pointer(
			instance.get(),
			&debug_messenger_creation_info,
			nullptr,
			&vulkan_debug_messenger
		) == VK_SUCCESS,
		"failed to create VkDebugUtilsMessengerEXT object",
		DebugContext::get()
	)
	creation_instance_pointer = &instance;
	return true;
}



[[nodiscard]] b8 DebugMessenger::basicDestroy() noexcept {
	debug_messenger_destruction_function_pointer(creation_instance_pointer->get(), vulkan_debug_messenger, nullptr);
	vulkan_debug_messenger = VK_NULL_HANDLE;
	return true;
}



[[nodiscard]] b8 DebugMessenger::debugExtensionSupported(const Instance& instance) const {
	PFN_vkCreateDebugUtilsMessengerEXT create_debug_messenger_function_pointer = (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance.get(), "vkCreateDebugUtilsMessengerEXT"));
	PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger_function_pointer = (PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance.get(), "vkDestroyDebugUtilsMessengerEXT"));
	return static_cast<b8>(create_debug_messenger_function_pointer) || static_cast<b8>(create_debug_messenger_function_pointer);
}



void DebugMessenger::findDebugMessengerCreationFunction(const Instance& instance) {
	FATAL_ASSERT(
		CURRENT_FUNCTION_SIGNATURE, 
		debugExtensionSupported(instance),
		"failed to find debug messenger creation function: debug utils extension not supported", 
		DebugContext::get()
	)
	debug_messenger_creation_function_pointer = (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance.get(), "vkCreateDebugUtilsMessengerEXT"));
}



void DebugMessenger::findDebugMessengerDestuctionFunction(const Instance& instance) {
	FATAL_ASSERT(
		CURRENT_FUNCTION_SIGNATURE, 
		debugExtensionSupported(instance), 
		"failed to find debug messenger destruction function: debug utils extension not supported",
		DebugContext::get()
	)
	debug_messenger_destruction_function_pointer = (PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance.get(), "vkDestroyDebugUtilsMessengerEXT"));
}



DebugMessenger::DebugMessenger(DebugMessenger&& other) noexcept :
	vulkan_debug_messenger(other.vulkan_debug_messenger)
{
	other.vulkan_debug_messenger = VK_NULL_HANDLE;
}



DebugMessenger& DebugMessenger::operator=(DebugMessenger&& other) noexcept {
	if (this == &other) {
		return *this;
	}
	vulkan_debug_messenger = other.vulkan_debug_messenger;
	other.vulkan_debug_messenger = VK_NULL_HANDLE;
	return *this;
}



DebugMessenger::~DebugMessenger() noexcept {
	destroy();
}



[[nodiscard]] b8 DebugMessenger::init(const Instance& instance, const VkDebugUtilsMessengerCreateInfoEXT& debug_messenger_creation_info) {
	return DebugContext::initWithDebug(
		[&]() { return basicInit(CURRENT_FUNCTION_SIGNATURE, instance, debug_messenger_creation_info); },
		[&]() { return isValid(); },
		1,
		"VkDebugUtilsMessengerEXT",
		CURRENT_FUNCTION_SIGNATURE
	);
}



[[nodiscard]] b8 DebugMessenger::init(
	const Instance& instance,
	VkDebugUtilsMessengerCreateFlagsEXT debug_message_creation_flags,
	VkDebugUtilsMessageSeverityFlagsEXT debug_message_severity_flags,
	VkDebugUtilsMessageTypeFlagsEXT debug_message_type,
	PFN_vkDebugUtilsMessengerCallbackEXT debug_messenger_callback,
	void* debug_messenger_user_data,
	const void* debug_messenger_next
) {
	VkDebugUtilsMessengerCreateInfoEXT new_debug_messenger_creation_info = {};
	new_debug_messenger_creation_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	new_debug_messenger_creation_info.pNext = debug_messenger_next;
	new_debug_messenger_creation_info.flags = debug_message_creation_flags;
	new_debug_messenger_creation_info.messageSeverity = debug_message_severity_flags;
	new_debug_messenger_creation_info.messageType = debug_message_type;
	new_debug_messenger_creation_info.pfnUserCallback = debug_messenger_callback;
	new_debug_messenger_creation_info.pUserData = debug_messenger_user_data;

	return DebugContext::initWithDebug(
		[&]() { return basicInit(CURRENT_FUNCTION_SIGNATURE, instance, new_debug_messenger_creation_info); },
		[&]() { return isValid(); },
		1,
		"VkDebugUtilsMessengerEXT",
		CURRENT_FUNCTION_SIGNATURE
	);
}



[[nodiscard]] b8 DebugMessenger::destroy() noexcept {
	return DebugContext::destroyWithDebug(
		[&]() { return basicDestroy(); },
		[&]() { return isValid(); },
		"VkDebugUtilsMessengerEXT",
		CURRENT_FUNCTION_SIGNATURE
	);
}


VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT debug_severity_flags,
	VkDebugUtilsMessageTypeFlagsEXT debug_message_type,
	const VkDebugUtilsMessengerCallbackDataEXT* debug_callback_data,
	void* debug_user_data
) {
	if (
		((debug_severity_flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) || (debug_severity_flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)) &&
		(debug_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
	) {
		LOG_INFO_NO_DEBUG_INFO(CURRENT_FUNCTION_SIGNATURE, debug_callback_data->pMessage)
	}
	else if (
		((debug_severity_flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) || (debug_severity_flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)) &&
		(debug_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) 
	) {
		LOG_WARNING_NO_DEBUG_INFO(CURRENT_FUNCTION_SIGNATURE, debug_callback_data->pMessage)
	}
	else if ((debug_severity_flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) && (debug_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)) {
		LOG_ERROR_NO_DEBUG_INFO(CURRENT_FUNCTION_SIGNATURE, debug_callback_data->pMessage)
	}
	else if ((debug_severity_flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) && (debug_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)) {
		LOG_FATAL_NO_DEBUG_INFO(CURRENT_FUNCTION_SIGNATURE, debug_callback_data->pMessage)
	}
	#ifdef VERBOSE
		else if (debug_message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
			LOG_INFO_NO_DEBUG_INFO(debug_callback_data->pMessage)
		}
	#endif

	return VK_FALSE;
}



VkDebugUtilsMessengerEXT DebugMessenger::operator*() const {
	FATAL_ASSERT(
		CURRENT_FUNCTION_SIGNATURE,
		isValid(),
		"attempted to acquire VkDebugUtilsMessengerEXT object which turned out to be VK_NULL_HANDLE", 
		DebugContext::get()
	)
	return vulkan_debug_messenger;
}



VkDebugUtilsMessengerEXT DebugMessenger::get() const {
	FATAL_ASSERT(
		CURRENT_FUNCTION_SIGNATURE, 
		isValid(),
		"attempted to acquire VkDebugUtilsMessengerEXT object which turned out to be VK_NULL_HANDLE", 
		DebugContext::get()
	)
		return vulkan_debug_messenger;
}



DebugMessenger::operator b8() const noexcept {
	return vulkan_debug_messenger != VK_NULL_HANDLE;
}



b8 DebugMessenger::isValid() const noexcept {
	return vulkan_debug_messenger != VK_NULL_HANDLE;
}