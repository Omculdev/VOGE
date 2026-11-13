#pragma once
#include "Utils/General/Logger.hpp"
#include "Utils/Macros/CurrentFunctionSignature.hpp"

#define VERBOSE
#define CONSOLE
#define CONSOLE_VERBOSE

#undef VERBOSE 
#undef CONSOLE_VERBOSE

#define FORMAT_LOG(func_sig, log_info, debug_info) std::string(func_sig) + ": " + std::string(log_info) + "; DebugInfo::ObjectId: " + std::to_string(debug_info.object_id.value()) + " DebugInfo::ObjectNumber: " + std::to_string(debug_info.object_number.value())
#define FORMAT_LOG_NO_DEBUG_INFO(func_sig, log_info) std::string(func_sig) + ": " + std::string(log_info) + ";"
#define FORMAT_LOG_AND_CHECK_FOR_NULLOPT(func, func_sig, log_info, debug_info) do { \
	if (!debug_info.object_id.has_value()) { \
		func(std::string(func_sig) + ": " + "failed to start log: DebugInfo::object_id has no value"); \
	} \
	else if (!debug_info.object_number.has_value()) { \
		func(std::string(func_sig) + ": " + "failed to start log: DebugInfo::object_number has no value"); \
	} \
	else { \
		func(FORMAT_LOG(func_sig, log_info, debug_info)); \
	}\
} while(false);

#ifdef NDEBUG
	#undef VERBOSE
	#undef CONSOLE

	#define LOG_INFO_IF(func_sig, condition, failure_log_info, failure_debug_info)
	#define WARNING_ASSERT(func_sig, condition, failure_log_info, failure_debug_info)
	#define LOG_INFO(func_sig, log_info, debug_info) 
	#define LOG_WARNING(func_sig, log_info, debug_info) 

	#define LOG_INFO_IF_NO_DEBUG_INFO(func_sig, condition, failure_log_info)
	#define WARNING_ASSERT_NO_DEBUG_INFO(func_sig, condition, failure_log_info)
	#define LOG_INFO_NO_DEBUG_INFO(func_sig, log_info) 
	#define LOG_WARNING_NO_DEBUG_INFO(func_sig, log_info) 
#else
	#define LOG_INFO(func_sig, log_info, debug_info) FORMAT_LOG_AND_CHECK_FOR_NULLOPT(Logger::getInstance().logInfo, func_sig, log_info, debug_info)
	#define LOG_WARNING(func_sig, log_info, debug_info) FORMAT_LOG_AND_CHECK_FOR_NULLOPT(Logger::getInstance().logWarning, func_sig, log_info, debug_info)
	#define LOG_INFO_IF(func_sig, condition, failure_log_info, failure_debug_info) do {\
			if (condition) { \
				LOG_INFO(func_sig, failure_log_info, failure_debug_info) \
			} \
		} while(false);
	#define WARNING_ASSERT(func_sig, condition, failure_log_info, failure_debug_info) do {\
			if (!condition) { \
				LOG_WARNING(func_sig, failure_log_info, failure_debug_info) \
			} \
		} while(false)

#define LOG_INFO_NO_DEBUG_INFO(func_sig, log_info) Logger::getInstance().logInfo(FORMAT_LOG_NO_DEBUG_INFO(func_sig, log_info));
#define LOG_WARNING_NO_DEBUG_INFO(func_sig, log_info) Logger::getInstance().logWarning(FORMAT_LOG_NO_DEBUG_INFO(func_sig, log_info));
#define LOG_INFO_IF_NO_DEBUG_INFO(func_sig, condition, failure_log_info) do {\
		if (condition) { \
			LOG_INFO_NO_DEBUG_INFO(func_sig, failure_log_info) \
		} \
	} while(false);
#define WARNING_ASSERT_NO_DEBUG_INFO(func_sig, condition, failure_log_info) do {\
		if (!condition) { \
			LOG_WARNING_NO_DEBUG_INFO(func_sig, failure_log_info) \
		} \
	} while(false)
#endif


#define LOG_ERROR(func_sig, log_info, debug_info) FORMAT_LOG_AND_CHECK_FOR_NULLOPT(Logger::getInstance().logError, func_sig,  log_info, debug_info)
#define LOG_FATAL(func_sig, log_info, debug_info) FORMAT_LOG_AND_CHECK_FOR_NULLOPT(Logger::getInstance().logFatal, func_sig,  log_info, debug_info)
#define THROW_AND_LOG_FATAL(func_sig, log_info, debug_info) \
Logger::getInstance().logFatal(FORMAT_LOG(func_sig, log_info, debug_info)); \
throw std::runtime_error(FORMAT_LOG(func_sig, log_info, debug_info)); 
#define ERROR_ASSERT(func_sig, condition, failure_log_info, failure_debug_info) do { \
		if (!condition) { \
			LOG_ERROR(func_sig, failure_log_info, failure_debug_info) \
		} \
	} while(false);
#define FATAL_ASSERT(func_sig, condition, failure_log_info, failure_debug_info) do { \
		if (!condition) { \
			THROW_AND_LOG_FATAL(func_sig, failure_log_info, failure_debug_info) \
		} \
	} while(false);

#define LOG_ERROR_NO_DEBUG_INFO(func_sig, log_info) Logger::getInstance().logError(FORMAT_LOG_NO_DEBUG_INFO(func_sig, log_info));
#define LOG_FATAL_NO_DEBUG_INFO(func_sig, log_info) Logger::getInstance().logWarning(FORMAT_LOG_NO_DEBUG_INFO(func_sig, log_info));
#define THROW_AND_LOG_FATAL_NO_DEBUG_INFO(func_sig, log_info) \
Logger::getInstance().logFatal(FORMAT_LOG_NO_DEBUG_INFO(func_sig, log_info)); \
throw std::runtime_error(FORMAT_LOG_NO_DEBUG_INFO(func_sig, log_info)); 
#define ERROR_ASSERT_NO_DEBUG_INFO(func_sig, condition, failure_log_info) do { \
		if (!condition) { \
			LOG_ERROR_NO_DEBUG_INFO(func_sig, failure_log_info) \
		} \
	} while(false);
#define FATAL_ASSERT_NO_DEBUG_INFO(func_sig, condition, failure_log_info) do { \
		if (!condition) { \
			THROW_AND_LOG_FATAL_NO_DEBUG_INFO(func_sig,failure_log_info) \
		} \
	} while(false);