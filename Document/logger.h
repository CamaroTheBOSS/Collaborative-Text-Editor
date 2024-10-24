#pragma once
#include <fstream>
#include <sstream>
#include <map>

namespace logs {
	enum class Level { error, info, debug };

	std::string lvlToStr(Level lvl);

	class Logger {
	public:
		Logger(std::string logFilePath);
		~Logger();

		template<typename... Args>
		void log(Level lvl, const Args... args) {
			time_t timestamp;
			time(&timestamp);
			std::stringstream stream;
			char timeBuffer[100];
			if (ctime_s(timeBuffer, 100, &timestamp)) {
				return;
			}
			std::string timeStr{timeBuffer};
			timeStr.erase(timeStr.size() - 1, 1);
			stream << "[" << timeStr << "] " << lvlToStr(lvl);
			([&] {
				stream << args;
				} (), ...);
			file << stream.str() << "\n" << std::flush;
		}
		template <typename...Args>
		void logInfo(const Args... args) {
			return log(Level::info, args...);
		}
		template <typename...Args>
		void logDebug(const Args... args) {
			return log(Level::debug, args...);
		}
		template <typename...Args>
		void logError(const Args... args) {
			return log(Level::error, args...);
		}
	private:
		std::ofstream file;
	};
}
#pragma once
