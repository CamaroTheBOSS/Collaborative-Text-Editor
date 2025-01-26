#pragma once
#include "Windows.h"
#include <string>

class LineModifier {
public:
	static SHORT insert(std::string& line, const int pos, const std::string_view newText);
	static int append(std::string& line, const std::string_view newText);
	static std::pair<int, std::string> erase(std::string& line, const int pos, const int n = INT_MAX);
	static std::string cut(std::string& line, const int left, const int right = INT_MAX);
	static std::string_view get(std::string& line, const int left, const int right = INT_MAX);
	static std::pair<int, int> effectiveRange(const std::string& line, const int left, const int right);
private:
	static bool endlPresent(const std::string& line);
};