#include <assert.h>

#include "text_container.h"

TextContainer::Cursor::Cursor() :
	pos(COORD{ 0, 0 }),
	offset(0),
	label(' ') {}

TextContainer::Cursor::Cursor(const COORD& pos, const int offset, const char label) :
	pos(pos),
	offset(offset),
	label(label) {}

void TextContainer::Cursor::shiftTo(const COORD& newPos, const TextContainer& doc) {
	auto& line = doc.getLine(newPos.Y);
	assert(newPos.X <= line.size());
	pos = newPos;
	offset = newPos.X;
	updateLabel(doc);
}

bool TextContainer::Cursor::isValid(const TextContainer& doc) const {
	auto& data = doc.get();
	return pos.Y >= 0 && pos.Y < data.size() && pos.X >= 0 && pos.X <= data[pos.Y].line.size();
}

void TextContainer::Cursor::updateLabel(const TextContainer& doc) {
	auto& line = doc.getLine(pos.Y);
	label = pos.X < line.size() ? line[pos.X] : ' ';
}

bool TextContainer::Cursor::operator==(const Cursor& other) const {
	return pos.X == other.pos.X && pos.Y == other.pos.Y && offset == other.offset && label == other.label;
}

COORD TextContainer::Cursor::moveLeft(const TextContainer& doc) {
	if (pos.X == 0 && pos.Y == 0) {
		return pos;
	}
	pos.X -= 1;
	if (pos.X < 0) {
		pos.Y -= 1;
		pos.X = doc.getLine(pos.Y).size();
	}
	offset = pos.X;
	updateLabel(doc);
	return pos;
}

COORD TextContainer::Cursor::moveRight(const TextContainer& doc) {
	if (pos.Y == doc.get().size() - 1 && pos.X == doc.getLine(pos.Y).size()) {
		return pos;
	}
	pos.X += 1;
	if (pos.X > doc.getLine(pos.Y).size()) {
		pos.Y += 1;
		pos.X = 0;
	}
	offset = pos.X;
	updateLabel(doc);
	return pos;
}

COORD TextContainer::Cursor::moveUp(const TextContainer& doc, const int width) {
	offset = offset % width;
	if (pos.Y == 0 && pos.X < width) {
		return pos;
	}
	if (pos.X >= width) {
		pos.X -= width;
	}
	else {
		pos.Y -= 1;
		int lineSize = doc.getLine(pos.Y).size();
		bool onBorder = lineSize % width == 0;
		int requestedX = (lineSize / width - onBorder) * width + offset;
		pos.X = (std::min)(requestedX, lineSize);
	}
	updateLabel(doc);
	return pos;
}

COORD TextContainer::Cursor::moveDown(const TextContainer& doc, const int width) {
	offset = offset % width;
	int lineSize = static_cast<int>(doc.getLine(pos.Y).size());
	bool onLastDocLine = pos.Y == doc.get().size() - 1;
	bool onLastRenderLine = pos.X / width == lineSize / width;
	bool noPlaceToMoveFully = pos.X >= lineSize - width;
	if (onLastDocLine && onLastRenderLine && noPlaceToMoveFully) {
		return pos;
	}
	if (pos.X <= lineSize - width) {
		pos.X += width;
	}
	else {
		pos.Y += 1;
		lineSize = doc.getLine(pos.Y).size();
		pos.X = (std::min)(offset, lineSize);
	}
	updateLabel(doc);
	return pos;
}

COORD TextContainer::Cursor::moveBy(const COORD& val, const TextContainer& doc) {	
	pos.Y = (std::min)(pos.Y + val.Y, static_cast<int>(doc.get().size()) - 1);
	pos.Y = (std::max)(pos.Y, static_cast<SHORT>(0));
	pos.X = (std::min)(pos.X + val.X, static_cast<int>(doc.getLine(pos.Y).size()));
	offset = pos.X;
	updateLabel(doc);
	return pos;
}

TextContainer::Row::Row() :
	line(""),
	endl(false),
	renderHeight(1) {}

TextContainer::Row::Row(const std::string& str) :
	line(str),
	endl(false),
	renderHeight(1) {}


TextContainer::TextContainer() :
	data({ Row() }),
	cursors({ Cursor() }),
	myCursorIdx(0) {}

TextContainer::TextContainer(const std::string& txt, std::vector<Cursor>& cursors, const int myCursorIdx) :
	data({ Row() }),
	cursors(std::move(cursors)),
	myCursorIdx(myCursorIdx) {
	insertText(COORD{ 0, 0 }, txt);
}

COORD TextContainer::write(Cursor& cursor, const std::string& txt) {
	assert(cursor.isValid(*this) && "Cursor has invalid position");
	assert(!txt.empty() && "Empty string cannot be written into document");
	auto cursorPos = insertText(cursor.pos, txt);
	COORD posDiff{ cursorPos.X - cursor.pos.X, cursorPos.Y - cursor.pos.Y };
	moveAffectedCursorsBy(posDiff, cursor.pos);
	return cursorPos;
}

void TextContainer::moveAffectedCursorsBy(const COORD& val, const COORD cursorPos) {
	for (auto& other : cursors) {
		if (other.pos.Y == cursorPos.Y && other.pos.X >= cursorPos.X) {
			other.moveBy(val, *this);
		}
		else if (other.pos.Y > cursorPos.Y) {
			other.moveBy(COORD{ 0, val.Y }, *this);
		}
	}
}

COORD TextContainer::insertText(COORD cursorPos, const std::string& txt) {
	size_t offset = 0;
	size_t end = txt.find('\n');
	if (end == std::string::npos) {
		data[cursorPos.Y].line.insert(cursorPos.X, txt.substr(offset, txt.size() - offset));
		cursorPos.X += txt.size() - offset;
	}
	else {
		size_t sizeToMoveDown = data[cursorPos.Y].line.size() - cursorPos.X;
		std::string toMoveDown = data[cursorPos.Y].line.substr(cursorPos.X, sizeToMoveDown);
		data[cursorPos.Y].line.erase(cursorPos.X, sizeToMoveDown);
		while (end != std::string::npos) {
			data[cursorPos.Y].line += txt.substr(offset, end - offset);
			addNewLine(cursorPos.Y);
			cursorPos.Y += 1;
			cursorPos.X = 0;
			offset = end + 1;
			end = txt.find('\n', offset);
		}
		data[cursorPos.Y].line += txt.substr(offset, txt.size() - offset) + toMoveDown;
		cursorPos.X += txt.size() - offset;
	}
	return cursorPos;
}

void TextContainer::addNewLine(const int col) {
	data[col].endl = true;
	data.insert(data.cbegin() + col + 1, Row());
}

COORD TextContainer::erase(Cursor& cursor, size_t size) {
	assert(cursor.isValid(*this) && "Cursor has invalid position");
	auto cursorPos = eraseText(cursor.pos, size);
	COORD posDiff{ cursorPos.X - cursor.pos.X, cursorPos.Y - cursor.pos.Y };
	moveAffectedCursorsBy(posDiff, cursor.pos);
	return cursorPos;
}

COORD TextContainer::eraseText(COORD cursorPos, size_t size) {
	if (size <= cursorPos.X) {
		cursorPos.X = cursorPos.X - size;
		data[cursorPos.Y].line.erase(cursorPos.X, size);
	}
	else {
		size_t sizeToMoveUp = data[cursorPos.Y].line.size() - cursorPos.X;
		std::string toMoveUp = data[cursorPos.Y].line.substr(cursorPos.X, sizeToMoveUp);
		data[cursorPos.Y].line.erase(cursorPos.X, sizeToMoveUp);
		do {
			size -= eraseLine(cursorPos.Y) + 1;
			cursorPos.Y -= 1;
		} while (cursorPos.Y >= 0 && size > data[cursorPos.Y].line.size());
		cursorPos.Y = (std::max)(cursorPos.Y, static_cast <SHORT>(0));
		auto& line = getLine(cursorPos.Y);
		size = (std::min)(size, line.size());
		data[cursorPos.Y].line.erase(line.cend() - size, line.cend());
		cursorPos.X = line.size();
		data[cursorPos.Y].line += toMoveUp;
	}
	return cursorPos;
}

size_t TextContainer::eraseLine(const int col) {
	size_t erasedSize = data[col].line.size();
	if (data.size() > 1) {
		data.erase(data.cbegin() + col);
	}
	if (col == data.size()) {
		data[col - 1].endl = false;
	}
	return erasedSize;
}

const std::string& TextContainer::getLine(const int idx) const {
	assert(idx >= 0 && idx < data.size() && "Invalid line index!");
	return data[idx].line;
}

const std::vector<TextContainer::Row>& TextContainer::get() const {
	return data;
}

TextContainer::Cursor& TextContainer::getCursor(const int idx) {
	assert(idx >= 0 && idx < cursors.size() && "Invalid cursor index!");
	return cursors[idx];
}

TextContainer::Cursor& TextContainer::getMyCursor() {
	return cursors[myCursorIdx];
}

void TextContainer::addCursor(Cursor& cursor) {
	assert(cursor.isValid(*this) && "Tried to add invalid cursor!");
	cursors.push_back(std::move(cursor));
}

void TextContainer::eraseCursor(const int idx) {
	assert(idx >= 0 && idx < data.size() && "Invalid line index!");
	assert(cursors[idx] != cursors[myCursorIdx] && "Tried to delete this client user's cursor");
	cursors.erase(cursors.cbegin() + idx);
	if (myCursorIdx > idx) {
		myCursorIdx -= 1;
	}
}