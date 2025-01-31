#include "text_container.h"
#include "line_modifier.h"
#include "pos_helpers.h"

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

COORD TextContainer::eraseBetween(const COORD& start, const COORD& end, std::vector<std::string>& erasedText) {
	auto [smaller, bigger] = getAscendingOrder(start, end);
	if (smaller->Y == bigger->Y) {
		auto [newX, line] = LineModifier::erase(data[smaller->Y], bigger->X, bigger->X - smaller->X);
		erasedText.emplace_back(std::move(line));
		return *smaller;
	}
	else {
		int sizeToMoveUp = (std::max)((int)data[bigger->Y].size() - bigger->X, 0);
		std::string toMoveUp = data[bigger->Y].substr(bigger->X, sizeToMoveUp);
		for (int i = 0; i < bigger->Y - smaller->Y; i++) {
			auto [size, line] = eraseLine(smaller->Y);
			erasedText.emplace_back(std::move(line));
		}
		auto [newX, line] = LineModifier::erase(data[smaller->Y], data[smaller->Y].size(), data[smaller->Y].size() - smaller->X);
		erasedText.emplace_back(std::move(line));
		data[smaller->Y] += toMoveUp;
	}
	return *smaller;
}