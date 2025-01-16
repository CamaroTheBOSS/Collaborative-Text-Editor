#pragma once
#include <fstream>
#include <sstream>
#include "messages.h"

namespace logs {
	
	enum class Level { error, info, debug };

	std::string lvlToStr(Level lvl);
	std::stringstream getCurrentTime();

	class Logger {
	public:
		Logger(std::string logFilePath);
		~Logger();

		template<typename... Args>
		void log(Level lvl, const Args... args) {
			std::stringstream stream = getCurrentTime();
			stream << lvlToStr(lvl);
			([&] {
				_log(stream, args);
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
		template<typename T>
		void _log(std::stringstream& ss, const T& arg) {
			ss << arg << " ";
		}
		void _log(std::stringstream& ss,  const msg::OneByteInt& arg) {
			ss << static_cast<int>(arg) << " ";
		}
		std::ofstream file;
	};
}
#pragma once
