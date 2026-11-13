#pragma once
#pragma once
#include <chrono>
#include <fstream>
#include <functional>
#include <ratio>
#include <string>
#include <type_traits>
#include <utility>
#include "Calculations/General/Types.hpp"

class Logger {
private:
	std::ofstream log_file = {};
	std::string log_file_title = {};

	std::string getCurrentSystemTime() const;

	void basicLog(const std::string& string, const std::string& additional);
	void consoleLog(const std::string& string, const std::string& additional);

	void createLogsFolder();
	void createLogsArchiveSubfolder();
	void moveOldLogsToArchive();

	Logger() = default;
	Logger(Logger& other) = delete;
	Logger& operator=(const Logger& other) = delete;
public:
	static inline Logger& getInstance() {
		static Logger logger;
		return logger;
	}



	static inline const Logger& getConstInstance() {
		return getInstance();
	}




	void init();

	void logInfo(const std::string& string);
	void logWarning(const std::string& string);
	void logError(const std::string& string);
	void logFatal(const std::string& string);
	template <typename Func, typename... Args>
	auto logPerformance(const std::string& string, Func&& func, Args&&... args) {
		using ReturnType = std::invoke_result_t<Func, Args...>;
		std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();

		if constexpr (!std::is_void_v<ReturnType>) {
			ReturnType returnvalue = std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

			std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
			f64 duration_milliseconds = std::chrono::duration<f64, std::milli>(end - start).count();

			basicLog(string + std::to_string(duration_milliseconds) + " milliseconds", " [PERFORMANCE] ");

			return returnvalue;
		}
		else {
			std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

			std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
			f64 duration_milliseconds = std::chrono::duration<f64, std::milli>(end - start).count();

			basicLog(string + std::to_string(duration_milliseconds) + " milliseconds", " [PERFORMANCE] ");
		}
	}
};