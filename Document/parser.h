#pragma once
#include <string>
#include <vector>

class Parser {
public:
	static std::vector<std::string> parseLineToVector(const std::string& line, const char delimiter = '\n');
	static std::vector<std::string> parseTextToVector(const std::string& text, const char delimiter = '\n');
	static std::string parseVectorToText(const std::vector<std::string>& vec, const char delimiter = '\n');
private:
	static void postprocess(std::vector<std::string>& vec);
};