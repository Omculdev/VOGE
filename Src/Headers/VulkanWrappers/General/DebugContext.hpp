#pragma once
#include "Calculations/General/Types.hpp"
#include "Utils/General/DebugInfo.hpp"
#include "Utils/Macros/DebugMacros.hpp"
#include <stacktrace>
#include <type_traits>
#include <functional>
#include <utility>
#include <optional>
#include "vulkan/vulkan.h"

template <typename DerivedWrapper>
class DebugContext {
private:
	DebugInfo debug_info = {};
	static inline u64 object_number = {};
protected:
	template <typename BasicInitFunc, typename ValidationFunc>
	requires std::same_as<b8, std::invoke_result_t<ValidationFunc>> && std::same_as<b8, std::invoke_result_t<BasicInitFunc>>
	[[nodiscard]] b8 initWithDebug(
		BasicInitFunc&& basic_init_func,
		ValidationFunc&& validation_func,
		u64 object_id,
		const std::string& object_name,
		const std::string& caller_sig
	) {
		debug_info.object_id = object_id;
		debug_info.object_number = object_number;
		object_number++;

		LOG_INFO(
			caller_sig, 
			"attempting to create " + object_name + " object", 
			debug_info
		)

		b8 object_invalid = !(std::invoke(std::forward<ValidationFunc>(validation_func)));
		FATAL_ASSERT(
			caller_sig,
			object_invalid,
			object_name + " object already initialized",
			debug_info
		)

		b8 object_created = std::invoke(std::forward<BasicInitFunc>(basic_init_func));
		if (!object_created) {
			LOG_ERROR(
				caller_sig,
				"failed to create " + object_name + " object: BasicInit function failed ",
				debug_info
			)
			return false;
		}

		LOG_INFO(
			caller_sig,
			"successfully created " + object_name + " object", 
			debug_info
		)

		return true;
	}



	template <typename BasicDestroyFunc, typename ValidationFunc>
	requires (std::same_as<b8, std::invoke_result_t<ValidationFunc>> && std::same_as<b8, std::invoke_result_t<BasicDestroyFunc>>)
	[[nodiscard]] b8 destroyWithDebug(
		BasicDestroyFunc&& basic_destroy_func,
		ValidationFunc&& validation_func,
		const std::string& object_name,
		const std::string& caller_sig
	) noexcept {
		b8 object_invalid = !(std::invoke(std::forward<ValidationFunc>(validation_func)));
		if (object_invalid) {
			LOG_ERROR_NO_DEBUG_INFO(
				caller_sig,
				"attempted to call destroy to an already deleted " + object_name + " object"
			)
			return false;
		}
		
		LOG_INFO(
			caller_sig,
			"attempting to destroy " + object_name + " object",
			debug_info
		)

		b8 object_destroyed = std::invoke(std::forward<BasicDestroyFunc>(basic_destroy_func));

		if (!object_destroyed) {
			LOG_ERROR(
				caller_sig,
				"failed to destroy " + object_name + " object: BasicDestroy function failed",
				debug_info
			)
			return false;
		}

		object_invalid = !(std::invoke(std::forward<ValidationFunc>(validation_func)));
		if (!object_invalid) {
			LOG_ERROR(
				caller_sig,
				"failed to destroy " + object_name + " object: BasicDestroy function was successful but the " + object_name + " object is not invalid",
				debug_info
			)
			return false;
		}
		
		LOG_INFO(
			caller_sig,
			"successfully destroyed " + object_name + " object",
			debug_info
		)

		debug_info.object_id = std::nullopt;
		debug_info.object_number = std::nullopt;
		return true;
	}

public:
	DebugContext() = default;
	DebugContext(const DebugContext& other) = delete;
	DebugContext& operator=(const DebugContext& other) = delete;
	DebugContext(DebugContext&& other) noexcept :
		debug_info(other.debug_info)
	{
		other.debug_info.object_id = std::nullopt;
		other.debug_info.object_number = std::nullopt;
	}



	DebugContext& operator=(DebugContext&& other) noexcept {
		if (this == &other) {
			return *this;
		}

		debug_info = other.debug_info;
		other.debug_info.object_id = std::nullopt;
		other.debug_info.object_number = std::nullopt;

		return *this;
	}



	~DebugContext() noexcept {
		debug_info.object_id = std::nullopt;
		debug_info.object_number = std::nullopt;
	}



	const DebugInfo& get() const {
		return debug_info;
	}



	DebugInfo& get() {
		return debug_info;
	}
};