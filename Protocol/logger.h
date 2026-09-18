#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include "messages.h"

namespace logs {
	
	enum class Level { none, error, info, debug, trace };

	std::string lvlToStr(Level lvl);
	Level strToLvl(std::string& str);
	std::stringstream getCurrentTime();

	class Logger {
	public:
		Logger(std::string logFilePath, bool stdOutLogs);
		~Logger();

		void setLogLevel(Level level) {
			_lvl = level;
		}

		template<typename... Args>
		void log(Level lvl, const Args... args) {
			if ((int)_lvl < (int)lvl) { return; }
			std::stringstream stream = getCurrentTime();
			stream << lvlToStr(lvl);
			([&] {
				_log(stream, args);
				} (), ...);
			file << stream.str() << "\n" << std::flush;
			if (stdOutLogs) {
				std::cout << stream.str() << "\n" << std::flush;
			}
			
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
		void logTrace(const Args... args) {
			return log(Level::trace, args...);
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
		template<typename T>
		void _log(std::stringstream& ss, const std::vector<T>& arg) {
			ss << "vector of " << typeid(T).name();
		}
		void _log(std::stringstream& ss,  const msg::OneByteInt& arg) {
			ss << static_cast<int>(arg) << " ";
		}
		std::ofstream file;
		Level _lvl;
		bool stdOutLogs;
	};
}
#pragma once
