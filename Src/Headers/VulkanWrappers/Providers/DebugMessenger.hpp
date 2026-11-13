#pragma once
#include "Calculations/General/Types.hpp"
#include "vulkan/vulkan.h"
#include "Utils/General/DebugInfo.hpp"
#include <string>
#include <span>
#include <optional>
#include "VulkanWrappers/Providers/Instance.hpp"
#include "VulkanWrappers/General/DebugContext.hpp"

class DebugMessenger : public DebugContext<DebugMessenger> {
private:
	friend class DebugContext<DebugMessenger>;

	const Instance* creation_instance_pointer = {};

	PFN_vkCreateDebugUtilsMessengerEXT debug_messenger_creation_function_pointer = {};
	PFN_vkDestroyDebugUtilsMessengerEXT debug_messenger_destruction_function_pointer = {};

	VkDebugUtilsMessengerEXT vulkan_debug_messenger = {};

	[[nodiscard]] b8 basicInit(
		const std::string& caller_sig,
		const Instance& instance, 
		const VkDebugUtilsMessengerCreateInfoEXT& debug_messenger_creation_info
	);
	[[nodiscard]] b8 basicDestroy() noexcept;

	[[nodiscard]] b8 debugExtensionSupported(const Instance& instance) const;
	void findDebugMessengerCreationFunction(const Instance& instance);
	void findDebugMessengerDestuctionFunction(const Instance& instance);
public:
	DebugMessenger() = default;
	DebugMessenger(const DebugMessenger& other) = delete;
	DebugMessenger& operator=(const DebugMessenger& other) = delete;
	DebugMessenger(DebugMessenger&& other) noexcept;
	DebugMessenger& operator=(DebugMessenger&& other) noexcept;
	~DebugMessenger() noexcept;

	[[nodiscard]] b8 init(const Instance& instace, const VkDebugUtilsMessengerCreateInfoEXT& debug_messenger_creation_info);
	[[nodiscard]] b8 init(
		const Instance& instance,
		VkDebugUtilsMessengerCreateFlagsEXT debug_message_creation_flags,
		VkDebugUtilsMessageSeverityFlagsEXT debug_message_severity_flags,
		VkDebugUtilsMessageTypeFlagsEXT debug_message_type,
		PFN_vkDebugUtilsMessengerCallbackEXT debug_messenger_callback,
		void* debug_messenger_user_data,
		const void* debug_messenger_next = nullptr
	);

	[[nodiscard]] b8 destroy() noexcept;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT debug_severity_flags,
		VkDebugUtilsMessageTypeFlagsEXT debug_message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* debug_callback_data,
		void* debug_user_data = nullptr
	);

	VkDebugUtilsMessengerEXT operator*() const;
	VkDebugUtilsMessengerEXT get() const;

	operator b8() const noexcept;
	b8 isValid() const noexcept;
};