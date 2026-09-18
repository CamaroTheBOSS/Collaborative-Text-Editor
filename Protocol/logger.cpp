#include <iomanip>
#include <chrono>

#include "logger.h"


namespace logs {
	using namespace std::chrono;

	std::string lvlToStr(Level lvl) {
		switch (lvl) {
		case Level::error:
			return "ERROR ";
		case Level::info:
			return "INFO ";
		case Level::debug:
			return "DEBUG ";
		case Level::trace:
			return "TRACE ";
		}
		return "UNDEFINED";
	}

	Level strToLvl(std::string& str) {
		if (str == "error") { return Level::error; }
		if (str == "info") { return Level::info; }
		if (str == "debug") { return Level::debug; }
		if (str == "trace") { return Level::trace; }
		return Level::info;
	}

	std::stringstream getCurrentTime() {
		auto highResNow = high_resolution_clock::now();
		auto fractionalPart = duration_cast<microseconds>(highResNow.time_since_epoch()).count() % 1000000;
		auto now = system_clock::now();
		auto time = system_clock::to_time_t(now);
		std::tm calendarTime;
		localtime_s(&calendarTime, &time);
		std::stringstream ss;
		ss << std::put_time(&calendarTime, "[ %d-%m-%Y %H:%M:%S.") << fractionalPart << " ] ";
		return ss;
	}

	Logger::Logger(std::string logFilePath, bool stdOutLogs) :
		file(logFilePath, std::ofstream::out | std::ofstream::trunc),
		stdOutLogs(stdOutLogs) {}

	Logger::~Logger() {
		file.close();
	}

}
