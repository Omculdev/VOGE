#include "VulkanWrappers/Providers/DebugMessenger.hpp"
#include "Utils/Macros/DebugMacros.hpp"
#include "vulkan/vulkan.h"



[[nodiscard]] b8 DebugMessenger::debugExtensionSupported(const Instance& instance) const {
	PFN_vkCreateDebugUtilsMessengerEXT create_debug_messenger_function_pointer = (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance.get(), "vkCreateDebugUtilsMessengerEXT"));
	PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger_function_pointer = (PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance.get(), "vkDestroyDebugUtilsMessengerEXT"));
	return static_cast<b8>(create_debug_messenger_function_pointer) || static_cast<b8>(create_debug_messenger_function_pointer);
}



void DebugMessenger::findDebugMessengerCreationFunction(const Instance& instance) {
	FATAL_ASSERT(debugExtensionSupported(instance), "failed to find debug messenger creation function: debug utils extension not supported", debug_info.value())
	debug_messenger_creation_function_pointer = (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance.get(), "vkCreateDebugUtilsMessengerEXT"));
}



void DebugMessenger::findDebugMessengerDestuctionFunction(const Instance& instance) {
	FATAL_ASSERT(debugExtensionSupported(instance), "failed to find debug messenger destruction function: debug utils extension not supported", debug_info.value())
	debug_messenger_destruction_function_pointer = (PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance.get(), "vkDestroyDebugUtilsMessengerEXT"));
}



DebugMessenger::DebugMessenger(DebugMessenger&& other) noexcept :
	vulkan_debug_messenger(other.vulkan_debug_messenger),
	debug_info(other.debug_info)
{
	other.vulkan_debug_messenger = VK_NULL_HANDLE;
	other.debug_info = std::nullopt;
}



DebugMessenger& DebugMessenger::operator=(DebugMessenger&& other) noexcept {
	if (this == &other) {
		return *this;
	}
	vulkan_debug_messenger = other.vulkan_debug_messenger;
	debug_info = other.debug_info;
	other.vulkan_debug_messenger = VK_NULL_HANDLE;
	other.debug_info = std::nullopt;
	return *this;
}



DebugMessenger::~DebugMessenger() noexcept {
	destroy();
}



void DebugMessenger::init(const Instance& instance, const VkDebugUtilsMessengerCreateInfoEXT& debug_messenger_creation_info) {
	debug_info = DebugInfo();
	debug_info.value().object_id = 1;
	debug_info.value().object_number = object_number;
	object_number++;

	LOG_INFO("attempting to create VkDebugUtilsMessengerEXT object", debug_info.value())
	FATAL_ASSERT(debugExtensionSupported(instance), "failed to create VkDebugUtilsMessengerEXT: debug utils vulkan extension unavaiable", debug_info.value())

	findDebugMessengerCreationFunction(instance);
	findDebugMessengerDestuctionFunction(instance);

	FATAL_ASSERT(
		debug_messenger_creation_function_pointer(
			instance.get(),
			&debug_messenger_creation_info,
			nullptr,
			&vulkan_debug_messenger
		) == VK_SUCCESS,
		"failed to create VkDebugUtilsMessengerEXT object", 
		debug_info.value()
	)

	LOG_INFO("successfully created VkDebugUtilsMessengerEXT object", debug_info.value());
}



void DebugMessenger::init(
	const Instance& instance,
	VkDebugUtilsMessengerCreateFlagsEXT debug_messenger_creation_flags,
	VkDebugUtilsMessageSeverityFlagsEXT debug_messenger_severity_flags,
	VkDebugUtilsMessageTypeFlagsEXT debug_messenger_messenge_type,
	PFN_vkDebugUtilsMessengerCallbackEXT debug_messenger_callback,
	void* debug_messenger_user_data,
	const void* debug_messenger_next
) {
	// do later
}



void DebugMessenger::destroy() noexcept {
	LOG_INFO("attempting to destroy VkDebugUtilsMessengerEXT object", debug_info.value())
	debug_messenger_destruction_function_pointer(creation_instance_pointer->get(), vulkan_debug_messenger, nullptr);
	LOG_INFO_IF(!isValid(), "successfully destroyed VkDebugUtilsMessengerEXT object", debug_info.value())
	ERROR_ASSERT(!isValid(), "failed to destroy VkDebugUtilsMessengerEXT object", debug_info.value())
}


VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT debug_severity_flags,
	VkDebugUtilsMessageTypeFlagsEXT debug_message_type,
	const VkDebugUtilsMessengerCallbackDataEXT* debug_callback_data,
	void* debug_user_data
) {
	// do later
}



VkDebugUtilsMessengerEXT DebugMessenger::operator*() const {
	FATAL_ASSERT(isValid(), "attempted to acquire VkDebugUtilsMessengerEXT object which turned out to be VK_NULL_HANDLE", debug_info.value())
	return vulkan_debug_messenger;
}



VkDebugUtilsMessengerEXT DebugMessenger::get() const {
	FATAL_ASSERT(isValid(), "attempted to acquire VkDebugUtilsMessengerEXT object which turned out to be VK_NULL_HANDLE", debug_info.value())
		return vulkan_debug_messenger;
}



DebugMessenger::operator b8() const noexcept {
	return vulkan_debug_messenger != VK_NULL_HANDLE;
}



b8 DebugMessenger::isValid() const noexcept {
	return vulkan_debug_messenger != VK_NULL_HANDLE;
}