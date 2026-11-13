#include "VulkanWrappers/Providers/Instance.hpp"
#include "Utils/Macros/DebugMacros.hpp"
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cstring>
#include <unordered_set> 
#include <numeric>



[[nodiscard]] b8 Instance::basicInit(const std::string& caller_sig, const VkInstanceCreateInfo& instance_creation_info) {
	u32 required_extension_count = instance_creation_info.enabledExtensionCount;
	const c8* const* required_extension_names = instance_creation_info.ppEnabledExtensionNames;
	std::vector<const c8*> required_extensions(required_extension_names, required_extension_names + required_extension_count);

	FATAL_ASSERT(
		caller_sig,
		extensionsSupported(required_extensions),
		"required extensions are not supported by current instance",
		DebugContext<Instance>::get()
	)

	FATAL_ASSERT(
		caller_sig,
		vkCreateInstance(
			&instance_creation_info,
			nullptr,
			&vulkan_instance
		) == VK_SUCCESS,
		"failed to create vkInstance object",
		DebugContext<Instance>::get()
	)

	return true;
}



[[nodiscard]] b8 Instance::basicDestroy() noexcept {
	vkDestroyInstance(vulkan_instance, nullptr);
	vulkan_instance = VK_NULL_HANDLE;

	return true;
}



[[nodiscard]] b8 Instance::vaildationLayersSupported(const std::vector<const c8*>& required_validation_layers) const {
	auto format_required_validation_layer_names_string = [&](const std::string& accumulation_string, const c8* item) -> std::string {
		return accumulation_string + "\n" + std::string(item);
	};
	std::string required_validation_layer_names_string = std::accumulate(
		required_validation_layers.begin(),
		required_validation_layers.end(), 
		std::string(""),
		format_required_validation_layer_names_string
	);

	LOG_INFO(
		CURRENT_FUNCTION_SIGNATURE,
		"checking for the availability of the following validation layers: " + required_validation_layer_names_string,
		DebugContext<Instance>::get()
	)

	u32 available_validation_layer_count = 0;
	vkEnumerateInstanceLayerProperties(&available_validation_layer_count, nullptr);

	std::vector<VkLayerProperties> available_validation_layers(available_validation_layer_count);
	vkEnumerateInstanceLayerProperties(&available_validation_layer_count, available_validation_layers.data());

	auto layer_properties_to_string = [&](const std::string& accumulation_string, const VkLayerProperties& layer_properties) -> std::string {
		return accumulation_string + "\n" + layer_properties.layerName;
	};
	std::string available_validation_layer_names_string = std::accumulate(
		available_validation_layers.begin(),
		available_validation_layers.end(),
		std::string(""),
		layer_properties_to_string
	);

	LOG_INFO(
		CURRENT_FUNCTION_SIGNATURE, 
		"found the following available validation layers:" + available_validation_layer_names_string,
		DebugContext<Instance>::get()
	)

	std::unordered_set<std::string> unique_required_layers(required_validation_layers.begin(), required_validation_layers.end());
	for (const auto& available_validation_layer : available_validation_layers) {
		unique_required_layers.erase(available_validation_layer.layerName);
	}

	auto format_missing_validation_layer_names_string = [&](const std::string& accumulation_string, const std::string& item) -> std::string {
		return accumulation_string + "\n" + item;
	};
	std::string missing_validation_layer_names_string = std::accumulate(
		unique_required_layers.begin(),
		unique_required_layers.end(),
		std::string(""),
		format_missing_validation_layer_names_string
	);

	if (unique_required_layers.empty()) {
		LOG_INFO(
			CURRENT_FUNCTION_SIGNATURE,
			"successfully found all of the following validation layers:" + required_validation_layer_names_string,
			DebugContext<Instance>::get()
		);
	}
	else {
		LOG_ERROR(
			CURRENT_FUNCTION_SIGNATURE, 
			"failed to find the following validation layers: " + missing_validation_layer_names_string,
			DebugContext<Instance>::get()
		)
	}

	return unique_required_layers.empty();
}



[[nodiscard]] b8 Instance::extensionsSupported(const std::vector<const c8*>& required_extensions) const {
	auto format_required_extension_names_string = [&](const std::string& accumulation_string, const c8* item) -> std::string {
		return accumulation_string + "\n" + std::string(item);
	};
	std::string required_extension_names_string = std::accumulate(
		required_extensions.begin(),
		required_extensions.end(),
		std::string(""),
		format_required_extension_names_string
	);

	LOG_INFO(CURRENT_FUNCTION_SIGNATURE, "checking for the availability of the following extensions: " + required_extension_names_string, DebugContext<Instance>::get())

	u32 available_extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);

	std::vector<VkExtensionProperties> available_extensions(available_extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extensions.data());

	auto extension_properties_to_string = [&](const std::string& accumulate_string, const VkExtensionProperties& extension_properties) -> std::string {
		return "\n" + accumulate_string + "\n" + extension_properties.extensionName;
	};
	std::string available_extension_names_string = std::accumulate(
		available_extensions.begin(),
		available_extensions.end(),
		std::string(""),
		extension_properties_to_string
	);

	LOG_INFO(CURRENT_FUNCTION_SIGNATURE, "found the following available extensions:" + available_extension_names_string, DebugContext<Instance>::get())

	std::unordered_set<std::string> unique_required_extensions(required_extensions.begin(), required_extensions.end());
	for (const auto& available_extension : available_extensions) {
		unique_required_extensions.erase(available_extension.extensionName);
	}

	auto format_missing_extension_names_string = [&](const std::string& accumulation_string, const std::string& item) -> std::string {
		return accumulation_string + "\n" + item;
	};
	std::string missing_extension_names_string = std::accumulate(
		unique_required_extensions.begin(),
		unique_required_extensions.end(),
		std::string(""),
		format_missing_extension_names_string
	);

	if (unique_required_extensions.empty()) {
		LOG_INFO(CURRENT_FUNCTION_SIGNATURE, "successfully found all of the following extensions:" + required_extension_names_string, DebugContext<Instance>::get());
	}
	else {
		LOG_ERROR(CURRENT_FUNCTION_SIGNATURE, "failed to find the following extensions: " + missing_extension_names_string, DebugContext<Instance>::get())
	}

	return unique_required_extensions.empty();
}



Instance::Instance(Instance&& other) noexcept :
	vulkan_instance(other.vulkan_instance),
	DebugContext<Instance>(std::move(other))
{
	other.vulkan_instance = VK_NULL_HANDLE;
}



Instance& Instance::operator=(Instance&& other) noexcept {
	if (this == &other) {
		return *this;
	}
	vulkan_instance = other.vulkan_instance;
	other.vulkan_instance = VK_NULL_HANDLE;
	DebugContext<Instance>::operator=(std::move(other));

	return *this;
}



Instance::~Instance() noexcept {
	destroy();
}


[[nodiscard]] b8 Instance::init(const VkInstanceCreateInfo& instance_creation_info) {
	return DebugContext<Instance>::initWithDebug(
		[&]() { return basicInit(CURRENT_FUNCTION_SIGNATURE, instance_creation_info); },
		[&]() { return isValid(); },
		0,
		"VkInstance",
		CURRENT_FUNCTION_SIGNATURE
	);
}



[[nodiscard]] b8 Instance::init(
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

	std::vector<const c8*> required_extensions_c_strings = {};
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

	return DebugContext<Instance>::initWithDebug(
		[&]() { return basicInit(CURRENT_FUNCTION_SIGNATURE, new_instance_creation_info); },
		[&]() { return isValid(); },
		0,
		"VkInstance",
		CURRENT_FUNCTION_SIGNATURE
	);
}



#ifndef NDEBUG
[[nodiscard]] b8 Instance::init(
	const VkInstanceCreateInfo& instance_creation_info,
	const VkDebugUtilsMessengerCreateInfoEXT& debug_messenger_creation_info,
	std::span<const std::string> required_validation_layers
) {
	std::vector<const c8*> required_validation_layers_c_strings = {};
	auto to_c_str = [&](const std::string& string) -> void {
		required_validation_layers_c_strings.emplace_back(string.c_str());
	};
	std::for_each(required_validation_layers.begin(), required_validation_layers.end(), to_c_str);

	FATAL_ASSERT(
		CURRENT_FUNCTION_SIGNATURE,
		vaildationLayersSupported(required_validation_layers_c_strings), 
		"required validation layers are not supported by current instance", 
		DebugContext<Instance>::get()
	);

	VkInstanceCreateInfo new_instance_creation_info = instance_creation_info;
	new_instance_creation_info.pNext = static_cast<const void*>(&debug_messenger_creation_info);
	new_instance_creation_info.enabledLayerCount = static_cast<u32>(required_validation_layers_c_strings.size());
	new_instance_creation_info.ppEnabledLayerNames = required_validation_layers_c_strings.data();

	return DebugContext<Instance>::initWithDebug(
		[&]() { return basicInit(CURRENT_FUNCTION_SIGNATURE, new_instance_creation_info); },
		[&]() { return isValid(); },
		0,
		"VkInstance",
		CURRENT_FUNCTION_SIGNATURE
	);
}

[[nodiscard]] b8 Instance::init(
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
	const void* application_info_next,
	const void* debug_messenger_next
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

	std::vector<const c8*> required_extensions_c_strings = {};
	auto extensions_to_c_str = [&](const std::string& string) -> void {
		required_extensions_c_strings.emplace_back(string.c_str());
		};
	std::for_each(required_extensions.begin(), required_extensions.end(), extensions_to_c_str);

	std::vector<const c8*> required_validation_layers_c_strings = {};
	auto validation_layers_to_c_str = [&](const std::string& string) -> void {
		required_validation_layers_c_strings.emplace_back(string.c_str());
	};
	std::for_each(required_validation_layers.begin(), required_validation_layers.end(), validation_layers_to_c_str);

	FATAL_ASSERT_NO_DEBUG_INFO(
		CURRENT_FUNCTION_SIGNATURE,
		vaildationLayersSupported(required_validation_layers_c_strings), 
		"required validation layers are not supported by current instance"
		"required validation layers are not supported by current instance"
	);

	VkDebugUtilsMessengerCreateInfoEXT new_debug_messenger_creation_info = {};
	new_debug_messenger_creation_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	new_debug_messenger_creation_info.pNext = debug_messenger_next;
	new_debug_messenger_creation_info.flags = debug_messenger_creation_flags;
	new_debug_messenger_creation_info.messageSeverity = debug_messenger_severity_flags;
	new_debug_messenger_creation_info.messageType = debug_messenger_messange_type;
	new_debug_messenger_creation_info.pfnUserCallback = debug_messenger_callback;
	new_debug_messenger_creation_info.pUserData = debug_messenger_user_data;

	VkInstanceCreateInfo new_instance_creation_info = {};
	new_instance_creation_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	new_instance_creation_info.pNext = static_cast<void*>(&new_debug_messenger_creation_info);
	new_instance_creation_info.flags = instance_creation_flags;
	new_instance_creation_info.pApplicationInfo = &new_application_info;
	new_instance_creation_info.enabledExtensionCount = static_cast<u32>(required_extensions_c_strings.size());
	new_instance_creation_info.ppEnabledExtensionNames = required_extensions_c_strings.data();
	new_instance_creation_info.enabledLayerCount = static_cast<u32>(required_validation_layers_c_strings.size());
	new_instance_creation_info.ppEnabledLayerNames = required_validation_layers_c_strings.data();

	return DebugContext<Instance>::initWithDebug(
		[&]() { return basicInit(CURRENT_FUNCTION_SIGNATURE, new_instance_creation_info); },
		[&]() { return isValid(); },
		0,
		"VkInstance",
		CURRENT_FUNCTION_SIGNATURE
	);
}
#endif



[[nodiscard]] b8 Instance::destroy() noexcept {
	return destroyWithDebug(
		[&]() { return basicDestroy(); },
		[&]() { return isValid(); },
		"VkInstance",
		CURRENT_FUNCTION_SIGNATURE
	);
}



[[nodiscard]] VkInstance Instance::operator*() const {
	FATAL_ASSERT(
		CURRENT_FUNCTION_SIGNATURE,
		isValid(),
		"attempted to acquire vkInstance object which turned out to be VK_NULL_HANDLE",
		DebugContext<Instance>::get()
	);
	return vulkan_instance;
}



[[nodiscard]] VkInstance Instance::get() const {
	FATAL_ASSERT(
		CURRENT_FUNCTION_SIGNATURE,
		isValid(),
		"attempted to acquire vkInstance object which turned out to be VK_NULL_HANDLE",
		DebugContext<Instance>::get()
	);
	return vulkan_instance;
}



Instance::operator b8() const noexcept {
	return vulkan_instance != VK_NULL_HANDLE;
}



[[nodiscard]] b8 Instance::isValid() const noexcept {
	return vulkan_instance != VK_NULL_HANDLE;
}
