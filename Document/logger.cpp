#include <ctime>
#include "logger.h"


namespace logs {

	std::string lvlToStr(Level lvl) {
		switch (lvl) {
		case Level::error:
			return "ERROR ";
		case Level::info:
			return "INFO ";
		case Level::debug:
			return "DEBUG ";
		}
		return "UNDEFINED";
	}

	Logger::Logger(std::string logFilePath) :
		file(logFilePath, std::ofstream::out | std::ofstream::trunc) {}

	Logger::~Logger() {
		file.close();
	}

}
