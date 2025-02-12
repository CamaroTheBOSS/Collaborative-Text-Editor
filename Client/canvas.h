#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <map>

#include "pos_helpers.h"

class Canvas {
public:
	Canvas(const COORD& size);
	COORD size() const;
	void resize(const COORD& size);
	void reset();
	void write(const std::string& data);
	void write(const std::string& data, const int color);
	void render() const;
	void setCursorPosition(const COORD& pos);

	static constexpr int defaultColor = 7;
private:
	void _render(const COORD& start, const COORD& end, const int color) const;
	void putColorInterval(const COORD& start, const COORD& end, const int color);
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	std::vector<std::vector<char>> _canvas;
	std::map<COORD, int> colorIntervals;
	COORD _size;
	COORD cursor;
};