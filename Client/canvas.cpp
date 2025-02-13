#include "canvas.h"
#include <iostream>
#include <algorithm>
#include <string_view>

Canvas::Canvas(const COORD& size) :
	_size(size),
	cursor(COORD{0, 0}) {
	reset();
}

COORD Canvas::size() const {
	return _size;
}

void Canvas::resize(const COORD& size) {
	_size = size;
	reset();
}

void Canvas::reset() {
	_canvas = std::vector<std::vector<char>>(_size.Y, std::vector<char>(_size.X, ' '));
	cursor = COORD{ 0, 0 };
	colorIntervals.clear();
	SetConsoleTextAttribute(hConsole, defaultColor);
}

void Canvas::write(const std::string& data) {
	int pos = 0;
	while (pos < data.size()) {
		auto& line = _canvas[cursor.Y];
		size_t length = (std::min)(data.size() - pos, line.size() - cursor.X);
		std::copy(std::begin(data), std::begin(data) + length, std::begin(line) + cursor.X);
		pos += length;
		cursor.X += length;
		if (cursor.X >= _canvas[cursor.Y].size()) {
			cursor.X == 0;
			cursor.Y++;
			if (cursor.Y >= _canvas[cursor.Y].size()) {
				cursor = _size;
				return;
			}
		}
	}
}

void Canvas::write(const std::string& data, const int color) {
	COORD start = cursor;
	write(data);
	putColorInterval(start, cursor, color);
}

void Canvas::putColorInterval(const COORD& start, const COORD& end, const int color) {
	if (start >= end) {
		return;
	}

	auto endUpperBound = colorIntervals.upper_bound(end);
	auto beforeEnd = endUpperBound != colorIntervals.end() ? std::prev(endUpperBound) : colorIntervals.end();

	int endValue = defaultColor;
	if (beforeEnd != colorIntervals.end()) {
		endValue = beforeEnd->second;
	}

	auto [startIt, startInserted] = colorIntervals.insert_or_assign(start, color);
	auto beforeStart = std::prev(startIt);
	auto [endIt, endInsertet] = colorIntervals.emplace(std::make_pair(end, endValue));
	auto afterEnd = std::next(endIt);

	bool startExtended = (beforeStart == colorIntervals.end() && color == defaultColor) ||
		(beforeStart != colorIntervals.end() && beforeStart->second == color);
	bool endExtended = (color == endValue) ||
		(afterEnd != colorIntervals.end() && afterEnd->second == endValue);

	auto startEraseIt = startExtended ? startIt : std::next(startIt);
	auto endEraseIt = endExtended ? afterEnd : endIt;
	colorIntervals.erase(startEraseIt, endEraseIt);
}

void Canvas::render() const {
	COORD start{ 0, 0 };
	int color = defaultColor;
	if (_canvas[0][20] == '-') {
		int x = 0;
	}
	for (const auto& [end, nextColor] : colorIntervals) {
		_render(start, end, color);
		color = nextColor;
		start = end;
		if (start.X >= _size.X) {
			start.X = 0;
			start.Y++;
		}
	}
	_render(start, _size - COORD{1, 1}, defaultColor);
}

void Canvas::_render(const COORD& start, const COORD& end, const int color) const {
	SetConsoleCursorPosition(hConsole, start);
	SetConsoleTextAttribute(hConsole, color);
	if (start.Y == end.Y) {
		auto line = std::string_view{ _canvas[start.Y].begin() + start.X, _canvas[start.Y].begin() + end.X };
		std::cout << line;
		return;
	}
	auto line = std::string_view{ _canvas[start.Y].begin() + start.X, _canvas[start.Y].end() };
	std::cout << line;
	for (int i = start.Y + 1; i < end.Y; i++) {
		line = std::string_view{ _canvas[i].begin(), _canvas[i].end() };
		std::cout << line;
	}
	line = std::string_view{ _canvas[end.Y].begin(), _canvas[end.Y].begin() + end.X };
	std::cout << line;
}

void Canvas::setCursorPosition(const COORD& pos) {
	cursor = pos;
}