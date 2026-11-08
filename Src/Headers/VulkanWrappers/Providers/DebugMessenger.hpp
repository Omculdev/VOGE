#pragma once
#include "Calculations/General/Types.hpp"
#include "vulkan/vulkan.h"
#include "Utils/General/DebugInfo.hpp"
#include <string>
#include <span>
#include <optional>
#include "VulkanWrappers/Providers/Instance.hpp"

class DebugMessenger {
private:
	static inline u64 object_number = {};
	std::optional<DebugInfo> debug_info = {};

	const Instance* creation_instance_pointer = {};

	PFN_vkCreateDebugUtilsMessengerEXT debug_messenger_creation_function_pointer = {};
	PFN_vkDestroyDebugUtilsMessengerEXT debug_messenger_destruction_function_pointer = {};

	VkDebugUtilsMessengerEXT vulkan_debug_messenger = {};

	void findDebugMessengerCreationFunction(const Instance& instance);
	void findDebugMessengerDestuctionFunction(const Instance& instance);
	[[nodiscard]] b8 debugExtensionSupported(const Instance& instance) const;
public:
	DebugMessenger() = default;
	DebugMessenger(const DebugMessenger& other) = delete;
	DebugMessenger& operator=(const DebugMessenger& other) = delete;
	DebugMessenger(DebugMessenger&& other) noexcept;
	DebugMessenger& operator=(DebugMessenger&& other) noexcept;
	~DebugMessenger() noexcept;

	void init(const Instance& instace, const VkDebugUtilsMessengerCreateInfoEXT& debug_messenger_creation_info);
	void init(
		const Instance& instance,
		VkDebugUtilsMessengerCreateFlagsEXT debug_messenger_creation_flags,
		VkDebugUtilsMessageSeverityFlagsEXT debug_messenger_severity_flags,
		VkDebugUtilsMessageTypeFlagsEXT debug_messenger_messenge_type,
		PFN_vkDebugUtilsMessengerCallbackEXT debug_messenger_callback,
		void* debug_messenger_user_data,
		const void* debug_messenger_next
	);

	void destroy() noexcept;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT debug_severity_flags,
		VkDebugUtilsMessageTypeFlagsEXT debug_message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* debug_callback_data,
		void* debug_user_data
	);

	VkDebugUtilsMessengerEXT operator*() const;
	VkDebugUtilsMessengerEXT get() const;

	operator b8() const noexcept;
	b8 isValid() const noexcept;
};