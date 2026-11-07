#include "VulkanWrappers/Providers/Instance.hpp"
#include "Utils/Macros/DebugMacros.hpp"
#include <stdexcept>
#include <vector>
#include <algorithm>
Instance::Instance(Instance&& other) noexcept :
	vulkan_instance(other.vulkan_instance),
	debug_info(other.debug_info)
{
	other.debug_info = std::nullopt;
	other.vulkan_instance = VK_NULL_HANDLE;
}

Instance& Instance::operator=(Instance&& other) noexcept {
	if (this == &other) return *this;
	vulkan_instance = other.vulkan_instance;
	debug_info = other.debug_info;
	other.debug_info = std::nullopt;
	other.vulkan_instance = VK_NULL_HANDLE;
	return *this;
}

Instance::~Instance() {
	destroy();
}

void Instance::init(const VkInstanceCreateInfo& instance_creation_info) {
	debug_info = DebugInfo();
	object_number++;

	LOG_INFO("attempting to create vkInstance object", debug_info.value());
	FATAL_ASSERT(!isValid(), "vkInstance object already initialized", debug_info.value())

	if (vkCreateInstance(&instance_creation_info, nullptr, &vulkan_instance) != VK_SUCCESS) {
		THROW_AND_LOG_FATAL("failed to create vkInstance object", debug_info.value())
	}

	LOG_INFO("successfully created vkInstance object", debug_info.value())
}

void Instance::init(
	const VkInstanceCreateInfo& instance_creation_info,
	const VkApplicationInfo& application_info
) {
	VkInstanceCreateInfo new_instance_creation_info = instance_creation_info;
	new_instance_creation_info.pApplicationInfo = &application_info;

	init(new_instance_creation_info);
}

void Instance::init(
	APIVersion api_version,
	ApplcationVersion application_version,
	EngineVersion engine_version,
	const std::string& application_name, const std::string& engine_name,
	VkInstanceCreateFlags instance_creation_flags,
	std::span<const std::string> required_extensions,
	const void* instance_creation_info_next, 
	const void* application_info_next
) {
	VkApplicationInfo new_application_info = {};
	new_application_info.pNext = application_info_next;
	new_application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	new_application_info.pApplicationName = application_name.c_str();
	new_application_info.applicationVersion = VK_MAKE_API_VERSION(
		application_version.variant,
		application_version.major,
		application_version.minor,
		application_version.patch
	);
	new_application_info.pEngineName = engine_name.c_str();
	new_application_info.engineVersion = VK_MAKE_API_VERSION(
		engine_version.variant,
		engine_version.major,
		engine_version.minor,
		engine_version.patch
	);
	new_application_info.apiVersion = VK_MAKE_API_VERSION(
		api_version.variant,
		api_version.major,
		api_version.minor,
		api_version.patch
	);

	std::vector<const char*> required_extensions_c_strings = {};
	auto to_c_str = [&](const std::string& string) -> void {
		required_extensions_c_strings.emplace_back(string.c_str());
	};
	std::for_each(required_extensions.begin(), required_extensions.end(), to_c_str);

	VkInstanceCreateInfo new_instance_creation_info = {};
	new_instance_creation_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	new_instance_creation_info.pNext = instance_creation_info_next;
	new_instance_creation_info.flags = instance_creation_flags;
	new_instance_creation_info.pApplicationInfo = &new_application_info;
	new_instance_creation_info.enabledExtensionCount = static_cast<u32>(required_extensions_c_strings.size());
	new_instance_creation_info.ppEnabledExtensionNames = required_extensions_c_strings.data();
	new_instance_creation_info.enabledLayerCount = 0;
	new_instance_creation_info.ppEnabledLayerNames = nullptr;

	init(new_instance_creation_info);
}

void Instance::destroy() {
	LOG_INFO("attempting to destroy vkInstance object", debug_info.value());

	vkDestroyInstance(vulkan_instance, nullptr);
	vulkan_instance = VK_NULL_HANDLE;

	ERROR_ASSERT(!isValid(), "failed to destroy vkInstance object", debug_info.value());
	LOG_INFO_IF(isValid(), "successfully destroyed vkInstance object", debug_info.value());
}

VkInstance Instance::operator*() const {
	FATAL_ASSERT(isValid(), "attempted to acquire vkInstance object which turned out to be VK_NULL_HANDLE", debug_info.value());
	return vulkan_instance;
}

VkInstance Instance::get() const {
	FATAL_ASSERT(isValid(), "attempted to acquire vkInstance object which turned out to be VK_NULL_HANDLE", debug_info.value());
	return vulkan_instance;
}

Instance::operator b8() const {
	return vulkan_instance != VK_NULL_HANDLE;
}

b8 Instance::isValid() const {
	return vulkan_instance != VK_NULL_HANDLE;
}
