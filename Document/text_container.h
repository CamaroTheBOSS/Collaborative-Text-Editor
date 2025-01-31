#pragma once
#include <Windows.h>
#include <vector>
#include <string>

class TextContainer {
public:
	TextContainer();
	TextContainer(const std::string& initText);
	TextContainer(std::vector<std::string>& initText);

	COORD insert(COORD pos, const std::vector<std::string>& parsedLines);
	COORD erase(COORD pos, int eraseSize, std::vector<std::string>& erasedText);
	COORD eraseBetween(const COORD& start, const COORD& end, std::vector<std::string>& erasedText);

	const std::string& addLine(const int col, const std::string& initText);
	std::pair<int, std::string> eraseLine(const int col);

	std::string getLine(const int col) const;
	std::string getText() const;
	std::string getTextBetween(const COORD pos1, const COORD pos2) const;
	int getLineSize(const int col) const;
	int getHeight() const;
	COORD getSize() const;
	COORD getEndPos() const;
	COORD getLineEndPos(const int col) const;
	COORD getStartPos() const;
	char getChar(const COORD pos) const;
	const std::vector<std::string>& get() const;

	bool empty() const;
	bool isPosValid(const COORD pos) const;
	COORD validatePos(COORD pos) const;

	TextContainer split(const COORD splitPoint);
	TextContainer& merge(TextContainer& second);
private:
	std::vector<std::string> data;
};