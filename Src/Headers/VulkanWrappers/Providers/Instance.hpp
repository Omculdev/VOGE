#pragma once
#include "Calculations/General/Types.hpp"
#include "vulkan/vulkan.h"
#include "Utils/General/DebugInfo.hpp"
#include <string>
#include <span>
#include <optional>
class Instance {
private:
	static inline u64 object_number = {};
	std::optional<DebugInfo> debug_info = {};

	VkInstance vulkan_instance = {};

	struct Version {
		u32 variant = {};
		u32 major = {};
		u32 minor = {};
		u32 patch = {};
	};
public:
	struct APIVersion : public Version {};
	struct ApplcationVersion : public Version {};
	struct EngineVersion : public Version {};

	struct ApplicationInfo {

	};
	Instance() = default;
	Instance(const Instance& other) = delete;
	Instance& operator=(const Instance& other) = delete;
	Instance(Instance&& other) noexcept;
	Instance& operator=(Instance&& other) noexcept;
	~Instance();

	void init(const VkInstanceCreateInfo& instance_creation_info);
	void init(
		const VkInstanceCreateInfo& instance_creation_info,
		const VkApplicationInfo& application_info
	);
	void init(
		APIVersion api_version,
		ApplcationVersion application_version,
		EngineVersion engine_version,
		const std::string& application_name, const std::string& engine_name,
		VkInstanceCreateFlags instance_creation_flags,
		std::span<const std::string> required_extensions,
		const void* instance_creation_info_next = nullptr, 
		const void* application_info_next = nullptr
	);

	void destroy();

	VkInstance operator*() const;
	VkInstance get() const;

	operator b8() const;
	b8 isValid() const;
};