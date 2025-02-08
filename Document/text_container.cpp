#include "text_container.h"
#include "line_modifier.h"
#include "pos_helpers.h"
#include "parser.h"

TextContainer::TextContainer() :
	data({ "" }) {};

TextContainer::TextContainer(const std::string& initText) :
	data({ "" }) {
	data.reserve(1024);
	auto parsedLines = Parser::parseTextToVector(initText);
	insert(COORD{ 0, 0 }, parsedLines);
};

TextContainer::TextContainer(std::vector<std::string>& initText) :
	data(std::move(initText)) {}

COORD TextContainer::insert(COORD pos, const std::vector<std::string>& parsedLines) {
	if (parsedLines.size() == 1) {
		pos.X = LineModifier::insert(data[pos.Y], pos.X, parsedLines[0]);
		return pos;
	}

	std::string toMoveDown = LineModifier::cut(data[pos.Y], pos.X);
	LineModifier::append(data[pos.Y], parsedLines[0]);
	for (int i = 1; i < parsedLines.size(); i++) {
		addLine(pos.Y + i, parsedLines[i]);
	}
	pos.Y += parsedLines.size() - 1;
	pos.X = data[pos.Y].size();
	data[pos.Y].append(toMoveDown);
	return pos;
}

const std::string& TextContainer::addLine(const int col, const std::string& initText) {
	data.insert(data.cbegin() + col, initText);
	return data[col];
}

COORD TextContainer::erase(COORD pos, int eraseSize, std::vector<std::string>& erasedText) {
	if (eraseSize < pos.X) {
		auto [newX, line] = LineModifier::erase(data[pos.Y], pos.X, eraseSize);;
		pos.X = newX;
		erasedText.emplace_back(std::move(line));
	}
	else {
		std::string toMoveUp = LineModifier::cut(data[pos.Y], pos.X);
		while (pos.Y > 0 && eraseSize > data[pos.Y].size()) {
			auto [size, line] = eraseLine(pos.Y--);
			eraseSize -= size;
			erasedText.emplace_back(std::move(line));
		}
		auto [newX, line] = LineModifier::erase(data[pos.Y], data[pos.Y].size(), eraseSize);
		LineModifier::append(data[pos.Y], toMoveUp);
		pos.X = newX;
		erasedText.emplace_back(std::move(line));
	}
	return pos;
}

std::pair<int, std::string> TextContainer::eraseLine(const int col) {
	int size = data[col].size() + 1;
	std::string line = data[col];
	data.erase(data.cbegin() + col);
	return { size, std::move(line) };
}

std::vector<std::string> TextContainer::eraseLines(const int start, const int end) {
	auto erasedLines = std::vector<std::string>(data.cbegin() + start, data.cbegin() + end + 1);
	data.erase(data.cbegin() + start, data.cbegin() + end + 1);
	return erasedLines;
}

COORD TextContainer::eraseBetween(const COORD& start, const COORD& end, std::vector<std::string>& erasedText) {
	auto [smaller, bigger] = getAscendingOrder(start, end);
	erasedText.reserve(bigger->Y - smaller->Y + 5);
	if (smaller->Y == bigger->Y) {
		auto [newX, line] = LineModifier::erase(data[smaller->Y], bigger->X, bigger->X - smaller->X);
		erasedText.emplace_back(std::move(line));
		return *smaller;
	}
	else {
		erasedText.emplace_back(LineModifier::cut(data[smaller->Y], smaller->X));
		data[smaller->Y] += LineModifier::cut(data[bigger->Y], bigger->X);
		auto erased = eraseLines(smaller->Y + 1, bigger->Y);
		for (const auto& element : erased) {
			erasedText.emplace_back(element);
		}
	}
	return *smaller;
}

std::string TextContainer::getLine(const int col) const {
	if (col >= getHeight()) {
		return "";
	}
	return data[col];
}

std::string TextContainer::getText() const {
	return Parser::parseVectorToText(data);
}

std::string TextContainer::getTextBetween(const COORD pos1, const COORD pos2) const {
	if (!isPosValid(pos1) || !isPosValid(pos2)) {
		return "";
	}
	auto [smaller, bigger] = getAscendingOrder(pos1, pos2);
	if (smaller->Y == bigger->Y) {
		return std::string{ LineModifier::get(data[smaller->Y], smaller->X, bigger->X) };
	}
	std::string text = std::string{ LineModifier::get(data[smaller->Y], smaller->X, getLineSize(smaller->Y)) } + "\n";
	for (int row = smaller->Y + 1; row < bigger->Y; row++) {
		text += data[row] + "\n";
	}
	text += LineModifier::get(data[bigger->Y], 0, bigger->X);
	return text;
}

int TextContainer::getLineSize(const int col) const {
	if (col < 0 || col >= getHeight()) {
		return -1;
	}
	return data[col].size();
}

int TextContainer::getHeight() const {
	return data.size();
}

COORD TextContainer::getSize() const {
	return COORD{ static_cast<SHORT>(getLineSize(data.size() - 1)),  static_cast<SHORT>(data.size()) };
}

COORD TextContainer::getEndPos() const {
	return getSize() - COORD{0, 1};
}

COORD TextContainer::getLineEndPos(const int col) const {
	SHORT lineSize = getLineSize(col);
	if (lineSize < 0) {
		return COORD{ -1, -1 };
	}
	return COORD{ lineSize, static_cast<SHORT>(col) };
}

COORD TextContainer::getStartPos() const {
	return COORD{ 0, 0 };
}

char TextContainer::getChar(const COORD pos) const {
	if (pos.Y < 0 || pos.Y >= getHeight() || pos.X < 0 || pos.X >= getLineSize(pos.Y)) {
		return ' ';
	}
	return data[pos.Y][pos.X];
}

const std::vector<std::string>& TextContainer::get() const {
	return data;
}

bool TextContainer::empty() const {
	return data.empty() || getHeight() == 1 && getLineSize(0) == 0;
}

bool TextContainer::isPosValid(const COORD pos) const {
	return pos.X >= 0 && pos.Y >= 0 && pos <= getLineEndPos(pos.Y);
}

COORD TextContainer::validatePos(COORD pos) const {
	if (isPosValid(pos)) {
		return pos;
	}
	if (pos < getStartPos()) {
		return getStartPos();
	}
	if (pos > getEndPos()) {
		return getEndPos();
	}
	if (pos.X < 0) {
		return COORD{ 0, pos.Y };
	}
	if (pos.X > getLineSize(pos.Y)) {
		return getLineEndPos(pos.Y);
	}
	return pos;
}

TextContainer TextContainer::split(const COORD splitPoint) {
	if (splitPoint > getSize() - COORD{1, 1} || splitPoint < COORD{0, 0} || splitPoint.X < 0) {
		return TextContainer();
	}
	std::vector<std::string> erased;
	eraseBetween(splitPoint, getEndPos(), erased);
	return TextContainer(erased);
}

TextContainer& TextContainer::merge(TextContainer& second) {
	insert(getEndPos(), second.get());
	return *this;
}