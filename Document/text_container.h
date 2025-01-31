#pragma once
#include <Windows.h>
#include <vector>
#include <string>

class TextContainer {
public:
	COORD insert(COORD pos, const std::vector<std::string>& parsedLines);
	COORD erase(COORD pos, int eraseSize, std::vector<std::string>& erasedText);
	COORD eraseBetween(const COORD& start, const COORD& end, std::vector<std::string>& erasedText);

	const std::string& addLine(const int col, const std::string& initText);
	std::pair<int, std::string> eraseLine(const int col);
private:
	std::vector<std::string> data;
};