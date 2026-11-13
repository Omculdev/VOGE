#pragma once
#include "Calculations/General/Types.hpp"
#include "vulkan/vulkan.h"
#include "Utils/General/DebugInfo.hpp"
#include "VulkanWrappers/General/DebugContext.hpp"
#include <string>
#include <span>
#include <vector>
#include <optional>

class Instance : public DebugContext<Instance> {
private:
	friend class DebugContext<Instance>;

	VkInstance vulkan_instance = {};

	struct Version {
		u32 variant = {};
		u32 major = {};
		u32 minor = {};
		u32 patch = {};
	};

	[[nodiscard]] b8 basicInit(const std::string& caller_sig, const VkInstanceCreateInfo& instance_creation_info);
	[[nodiscard]] b8 basicDestroy() noexcept;

	[[nodiscard]] b8 vaildationLayersSupported(const std::vector<const c8*>& required_validation_layers) const;
	[[nodiscard]] b8 extensionsSupported(const std::vector<const c8*>& required_extensions) const;
public:
	struct APIVersion : public Version {};
	struct ApplcationVersion : public Version {};
	struct EngineVersion : public Version {};

	Instance() = default;
	Instance(const Instance& other) = delete;
	Instance& operator=(const Instance& other) = delete;
	Instance(Instance&& other) noexcept;
	Instance& operator=(Instance&& other) noexcept;
	~Instance() noexcept;

	[[nodiscard]] b8 init(const VkInstanceCreateInfo& instance_creation_info);
	[[nodiscard]] b8 init(
		APIVersion api_version,
		ApplcationVersion application_version,
		EngineVersion engine_version,
		const std::string& application_name, const std::string& engine_name,
		VkInstanceCreateFlags instance_creation_flags,
		std::span<const std::string> required_extensions,
		const void* instance_creation_info_next = nullptr, 
		const void* application_info_next = nullptr
	);
#ifndef NDEBUG
	[[nodiscard]] b8 init(
		const VkInstanceCreateInfo& instance_creation_info,
		const VkDebugUtilsMessengerCreateInfoEXT& debug_messenger_creation_info, 
		std::span<const std::string> required_validation_layers
	);
	[[nodiscard]] b8 init(
		APIVersion api_version,
		ApplcationVersion application_version,
		EngineVersion engine_version,
		const std::string& application_name, const std::string& engine_name,
		VkInstanceCreateFlags instance_creation_flags,
		std::span<const std::string> required_extensions,
		std::span<const std::string> required_validation_layers,
		VkDebugUtilsMessengerCreateFlagsEXT debug_messenger_creation_flags,
		VkDebugUtilsMessageSeverityFlagsEXT debug_messenger_severity_flags,
		VkDebugUtilsMessageTypeFlagsEXT debug_messenger_messange_type,
		PFN_vkDebugUtilsMessengerCallbackEXT debug_messenger_callback,
		void* debug_messenger_user_data,
		const void* application_info_next = nullptr,
		const void* debug_messenger_next = nullptr
	);
#endif

	[[nodiscard]] b8 destroy() noexcept;

	[[nodiscard]] VkInstance operator*() const;
	[[nodiscard]] VkInstance get() const;

	operator b8() const noexcept;
	[[nodiscard]] b8 isValid() const noexcept;
};