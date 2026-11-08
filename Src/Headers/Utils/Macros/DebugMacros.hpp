#pragma once
#include "Utils/General/Logger.hpp"
#include "Utils/Macros/CurrentFunctionSignature.hpp"
#define FORMAT_LOG(log_info, debug_info) std::string(CURRENT_FUNCTION_SIGNATURE) + ": " + std::string(log_info) + "; DebugInfo::ObjectId: " + std::to_string(debug_info.object_id) + " DebugInfo::ObjectNumber: " + std::to_string(debug_info.object_number)
#ifdef NDEBUG
#define LOG_INFO_IF(condition, failure_log_info, failure_debug_info)
#define WARNING_ASSERT(condition, failure_log_info, failure_debug_info)
#define LOG_INFO(log_info, debug_info) 
#define LOG_WARNING(log_info, debug_info) 
#else
#define LOG_INFO(log_info, debug_info) Logger::getInstance().logInfo(FORMAT_LOG(log_info, debug_info));
#define LOG_WARNING(log_info, debug_info) Logger::getInstance().logWarning(FORMAT_LOG(log_info, debug_info));
#define LOG_INFO_IF(condition, failure_log_info, failure_debug_info) do {\
	if (condition) { \
		LOG_INFO(failure_log_info, failure_debug_info) \
	} \
} while(false);
#define WARNING_ASSERT(condition, failure_log_info, failure_debug_info) do {\
	if (!condition) { \
		LOG_WARNING(failure_log_info, failure_debug_info) \
	} \
} while(false)
#endif
#define LOG_ERROR(log_info, debug_info) Logger::getInstance().logError(FORMAT_LOG(log_info, debug_info));
#define THROW_AND_LOG_FATAL(log_info, debug_info) \
Logger::getInstance().logFatal(FORMAT_LOG(log_info, debug_info)); \
throw std::runtime_error(FORMAT_LOG(log_info, debug_info)); 
#define ERROR_ASSERT(condition, failure_log_info, failure_debug_info) do { \
	if (!condition) { \
		LOG_ERROR(failure_log_info, failure_debug_info) \
	} \
} while(false);
#define FATAL_ASSERT(condition, failure_log_info, failure_debug_info) do { \
	if (!condition) { \
		THROW_AND_LOG_FATAL(failure_log_info, failure_debug_info) \
	} \
} while(false);