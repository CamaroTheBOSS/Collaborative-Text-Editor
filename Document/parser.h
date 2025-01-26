#pragma once
#include <string>
#include <vector>

class Parser {
public:
	static std::vector<std::string> parseTextToVector(const std::string& text);
	static std::string parseVectorToText(const std::vector<std::string>& vec);
};