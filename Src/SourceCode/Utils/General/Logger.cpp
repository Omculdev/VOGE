#include "Utils/General/Logger.hpp"
#include <fstream>
#include <format>
#include <iostream>
#include <windows.h>
#include <filesystem>
std::string Logger::getCurrentSystemTime() const {
	std::chrono::time_point now = std::chrono::system_clock::now();
	return std::format("{:%Y-%m-%d %H-%M-%S}", now);
}
void Logger::basicLog(const std::string& logdata, const std::string& additional) {
	std::string logstring = "<" + getCurrentSystemTime() + ">" + additional + logdata;
	log_file << logstring << "\n";
}
void Logger::logInfo(const std::string& logdata) {
	basicLog(logdata, " [INFO] ");
}
void Logger::logWarning(const std::string& logdata) {
	basicLog(logdata, " [WARNING] ");
}
void Logger::logFatal(const std::string& logdata) {
	basicLog(logdata, " [FATAL] ");
}
void Logger::logError(const std::string& logdata) {
	basicLog(logdata, " [ERROR] ");
}
void Logger::createLogsFolder() {
	logInfo("createLogsFolder: Attemptring to create logs folder");
	try {
		std::filesystem::create_directories("logs");
	}
	catch (std::filesystem::filesystem_error& exception) {
		std::cerr << "createLogsFolder: Failed to create logs folder" + std::string(exception.what());
		throw;
	}
	logInfo("createLogsFolder: Successfully created logs folder");
}
void Logger::createLogsArchiveSubfolder() {
	logInfo("createLogsArchiveSubfolder: Attemptring to create logs archive subfolder");
	try {
		std::filesystem::create_directories("logs/archive");
	}
	catch (std::filesystem::filesystem_error& exception) {
		std::cerr << "createLogsArchiveSubfolder: Failed to create logs archive subfolder" + std::string(exception.what());
		throw;
	}
	logInfo("createLogsArchiveSubfolder: Successfully created logs archive subfolder");
}
void Logger::moveOldLogsToArchive() {
	if (!std::filesystem::exists("logs")) {
		std::cerr << "moveOldLogsToArchive: Logs folder not found";
		createLogsFolder();
	}
	if (!std::filesystem::exists("logs/archive")) {
		std::cerr << "moveOldLogsToArchive: Logs archive subfolder not found";
		createLogsArchiveSubfolder();
	}
	for (const auto& file : std::filesystem::directory_iterator("logs")) {
		if (file.is_regular_file() && file.path().extension() == ".log") {
			rename(file.path(), "logs/archive/" + file.path().filename().string());
		}
	}
}
void Logger::init() {
	createLogsFolder();
	createLogsArchiveSubfolder();
	moveOldLogsToArchive();
	std::chrono::time_point now = std::chrono::system_clock::now();
	log_file_title = "GraphicsEngineLog " + getCurrentSystemTime() + ".log";
	log_file.open("logs/" + log_file_title, std::ios::app);
	if (!log_file.is_open()) {
		std::cerr << "init failed: Failed to create log file: " + log_file_title;
		return;
	}
}