#pragma once
#include "Windows.h"
#include <string>

class LineModifier {
public:
	static SHORT insert(std::string& line, const int pos, const std::string_view newText);
	static int append(std::string& line, const std::string_view newText);
	static int erase(std::string& line, const int pos, const int n = INT_MAX);
	static std::string cut(std::string& line, const int left, const int right = INT_MAX);
	static std::string_view get(std::string& line, const int left, const int right = INT_MAX);
	static std::pair<int, int> effectiveRange(std::string& line, const int left, const int right);
};