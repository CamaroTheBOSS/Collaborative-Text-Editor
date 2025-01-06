#include "new_text_container.h"

namespace v3 {

	bool operator>(const Pos& first, const Pos& other) {
		if (first.y > other.y || (first.y == other.y && first.x > other.x)) {
			return true;
		}
		return false;
	}
	bool operator<(const Pos& first, const Pos& other) {
		if (first.y < other.y || (first.y == other.y && first.x < other.x)) {
			return true;
		}
		return false;
	}
	bool operator==(const Pos& first, const Pos& other) {
		return first.x == other.x && other.y == first.y;
	}
	bool operator>=(const Pos& first, const Pos& other) {
		return first > other || first == other;
	}
	bool operator<=(const Pos& first, const Pos& other) {
		return first < other || first == other;
	}
	Pos& Pos::operator=(const Pos& other) {
		x = other.x;
		y = other.y;
		return *this;
	}
	Pos& Pos::operator+(const Pos& other) {
		x += other.x;
		y += other.y;
		return *this;
	}
	Pos& Pos::operator-(const Pos& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	Cursor::Cursor(const Pos& pos) :
		pos(pos),
		offset(pos.x),
		label(' ') {}

	Cursor::Cursor(const Pos& pos, const int offset, const char label):
		pos(pos),
		offset(offset),
		label(label) {}

	bool operator==(const Cursor& first, const Cursor& other) {
		return first.pos == other.pos && first.offset == other.offset && first.label == other.label;
	}

	void Cursor::updateAfterEdit(const Pos& newPos, const std::string_view lastLine) {
		pos = newPos;
		offset = newPos.x;
		updateLabel(lastLine);
	}

	void Cursor::updateLabel(const std::string_view relevantLine) {
		label = (pos.x >= relevantLine.size() || relevantLine[pos.x] == '\n') ? ' ' : relevantLine[pos.x];
	}

	Line::Line(const std::string_view& text) :
		text(text) {}

	int Line::insert(const int pos, const std::string_view newText) {
		if (pos < 0 || pos > size() || newText.empty()) {
			return pos;
		}
		text.insert(pos, std::string{ newText });
		return pos + newText.size();
	}

	int Line::append(const std::string_view newText) {
		text.append(std::string{ newText });
		return size();
	}

	int Line::erase(const int pos, const int n = INT_MAX) {
		if (pos <= 0 || pos > size() || n <= 0) {
			return pos;
		}
		const int nLetters = std::min(pos, n);
		text.erase(text.cbegin() + pos - nLetters, text.cbegin() + pos);
		return pos - nLetters;
	}

	std::string Line::cut(int left, int right = INT_MAX) {
		if (left >= right || text.empty()) {
			return "";
		}
		left = effectiveLeft(left);
		right = effectiveRight(right);
		std::string str{text.cbegin() + left, text.cbegin() + right};
		text.erase(text.cbegin() + left, text.cbegin() + right);
		return str;
	}

	std::string_view Line::get(const int left, const int right = INT_MAX) const {
		if (left >= right || text.empty()) {
			return "";
		}
		return std::string_view{text.cbegin() + effectiveLeft(left), text.cbegin() + effectiveRight(right)};
	}
	int Line::size() const {
		return text.size();
	}
	int Line::effectiveLeft(const int left) const {
		return std::max(0, left);
	}
	int Line::effectiveRight(const int right) const {
		return std::min(size(), right);
	}

	NewTextContainer::NewTextContainer(const std::string& text) {
		auto parsedLines = parseText(text);
		insertText(Pos{ 0, 0 }, parsedLines);
	}

	Pos& NewTextContainer::write(Cursor& cursor, const std::string& newText) {
		if (!isCursorValid(cursor)) {
			return cursor.pos;
		}
		auto parsedLines = parseText(newText);	
		Pos endPos = insertText(cursor.pos, parsedLines);
		cursor.updateAfterEdit(endPos, lines[cursor.pos.y + parsedLines.size() - 1].get(0));
		return cursor.pos;
	}

	Pos NewTextContainer::insertText(Pos pos, const std::vector<std::string_view>& parsedLines) {
		if (parsedLines.size() == 1) {
			pos.x = lines[pos.y].insert(pos.x, parsedLines[0]);
			return pos;
		}

		std::string toMoveDown = lines[pos.y].cut(pos.x);
		lines[pos.y].append(parsedLines[0]);
		for (int i = 1; i < parsedLines.size(); i++) {
			addNewLine(pos.y + i, parsedLines[i]);
		}
		pos.y += parsedLines.size() - 1;
		pos.x = lines[pos.y].size();
		lines[pos.y].append(toMoveDown);
		return pos;
	}

	Line& NewTextContainer::addNewLine(const int col, const std::string_view initText) {
		lines.insert(lines.cbegin() + col, Line{initText});
		return lines[col];
	}

	std::vector<std::string_view> NewTextContainer::parseText(const std::string& text) const {
		size_t offset = 0;
		size_t end = text.find('\n');
		std::vector<std::string_view> parsedLines;
		if (end == std::string::npos) {
			parsedLines.push_back( std::string_view{ text });
			return parsedLines;
		}
		do {
			std::string_view lineText{text.cbegin() + offset, text.cbegin() + end};
			parsedLines.push_back(lineText);
			offset = end + 1;
			end = text.find('\n', offset);
		} while (end != std::string::npos);
		std::string_view lineText{text.cbegin() + offset, text.cend()};
		parsedLines.push_back(lineText);
		return parsedLines;
	}

	Pos& NewTextContainer::erase(Cursor& cursor, int n) {
		if (!isCursorValid(cursor)) {
			return cursor.pos;
		}

		Pos endPos = cursor.pos;
		if (n < cursor.pos.x) {
			endPos.x = lines[cursor.pos.y].erase(cursor.pos.x, n);
		}
		else {
			std::string toMoveUp = lines[endPos.y].cut(endPos.x);
			while (endPos.y > 0 && n > lines[endPos.y].size()) {
				n -= eraseLine(endPos.y--);
			}
			endPos.x = lines[endPos.y].erase(lines[endPos.y].size(), n);
			lines[endPos.y].append(toMoveUp);
		}
		cursor.updateAfterEdit(endPos, lines[endPos.y].get(0));
		return endPos;
	}

	int NewTextContainer::eraseLine(const int col) {
		int size = lines[col].size() + 1;
		lines.erase(lines.cbegin() + col);
		return size;
	}

	Pos& NewTextContainer::moveLeft(Cursor& cursor) {
		if (cursor.pos <= startPos()) {
			cursor.pos = startPos();
		}
		else if (cursor.pos.x <= 0) {
			cursor.pos.y = std::min(cursor.pos.y, size()) - 1;
			cursor.pos.x = lines[cursor.pos.y].size();
		}
		else {
			cursor.pos.x -= 1;
		}
		cursor.offset = cursor.pos.x;
		cursor.updateLabel(lines[cursor.pos.y].get(0));
		return cursor.pos;
	}

	Pos& NewTextContainer::moveRight(Cursor& cursor) {
		if (cursor.pos >= endPos()) {
			cursor.pos = endPos();
		}
		else if (cursor.pos.x >= lines[cursor.pos.y].size()) {
			cursor.pos.y += 1;
			cursor.pos.x = 0;
		}
		else {
			cursor.pos.x += 1;
		}
		cursor.offset = cursor.pos.x;
		cursor.updateLabel(lines[cursor.pos.y].get(0));
		return cursor.pos;
	}

	Pos& NewTextContainer::moveUp(Cursor& cursor, const int width) {
		int offset = cursor.offset % width;
		if (cursor.pos <= Pos{ width, 0 }) {
			cursor.pos = startPos();
		}
		else if (cursor.pos.x < width) {
			cursor.pos.y -= 1;
			int lineSize = lines[cursor.pos.y].size();
			bool onBorder = lineSize % width == 0;
			int requestedX = (lineSize / width - onBorder) * width + offset;
			cursor.pos.x = (std::min)(requestedX, lineSize);
		}
		else {
			cursor.pos.x -= width;
		}
		cursor.updateLabel(lines[cursor.pos.y].get(0));
		return cursor.pos;
	}

	Pos& NewTextContainer::moveDown(Cursor& cursor, const int width) {
		int offset = cursor.offset % width;
		if (cursor.pos >= endPos() - Pos{ width, 0 }) {
			cursor.pos = endPos();
		}
		else if (cursor.pos.x > lines[cursor.pos.y].size() - width) {
			cursor.pos.y += 1;
			cursor.pos.x = (std::min)(offset, lines[cursor.pos.y].size());
		}
		else {
			cursor.pos.x += width;
		}
		cursor.updateLabel(lines[cursor.pos.y].get(0));
		return cursor.pos;
	}

	Pos& NewTextContainer::moveTo(Cursor& cursor, const Pos& pos) {
		cursor.pos = cursor.pos + pos;
		if (cursor.pos < startPos()) {
			cursor.pos = startPos();
		}
		else if (cursor.pos > endPos()) {
			cursor.pos = endPos();
		}
		cursor.offset = cursor.pos.x;
		cursor.updateLabel(lines[cursor.pos.y].get(0));
		return cursor.pos;
	}

	std::string NewTextContainer::get(const Pos& left, const Pos& right) const {
		std::string txt{ lines[left.y].get(left.x) };
		for (int y = left.y; y < right.y; y++) {
			txt += lines[y].get(0);
		}
		txt += lines[right.y].get(0, right.x);
		return txt;
	}

	std::string NewTextContainer::getLine(const int line) const {
		return std::string{ lines[line].get(0) };
	}

	Pos NewTextContainer::startPos() const {
		return Pos{ 0, 0 };
	}

	Pos NewTextContainer::endPos() const {
		return Pos{ lines[size() - 1].size(), size() - 1};
	}

	int NewTextContainer::size() const {
		return lines.size();
	}

	bool NewTextContainer::isCursorValid(const Cursor& cursor) const {
		return cursor.pos.y < size() && cursor.pos.x <= lines[cursor.pos.y].size();
	}

}