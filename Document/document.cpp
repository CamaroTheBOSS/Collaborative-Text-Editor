#include <algorithm>
#include "document.h"

#define NOMINMAX

Document::Document():
	data({""}),
	cursors({ Cursor() }),
	myCursorIdx(0) {
	data.reserve(1024);
}

Document::Document(const std::string& text):
	data(),
	cursors({ Cursor() }) {
	data.reserve(text.size());
	setText(text);
}

Document::Document(const std::string& text, const int nCursors, const int myCursorIdx) :
	data(),
	myCursorIdx(myCursorIdx),
	cursors(nCursors, Cursor()) {
	data.reserve(text.size());
	setText(text);
}

COORD Document::write(const int index, const char letter) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	auto cursorPos = cursors[index].position();
	if (cursorPos.Y > data.size()) {
		return cursorPos;
	}
	if (cursorPos.X >= data[cursorPos.Y].size()) {
		data[cursorPos.Y] += letter;
		cursorPos.X += 1;
		if (letter == '\n') {
			data.insert(data.begin() + cursorPos.Y + 1, "");
			cursorPos.Y += 1;
			cursorPos.X = 0;
		}
	}
	else {
		data[cursorPos.Y].insert(data[cursorPos.Y].begin() + cursorPos.X, letter);
		cursorPos.X += 1;
		if (letter == '\n') {
			std::string toMoveBelow = data[cursorPos.Y].substr(cursorPos.X, data[cursorPos.Y].size() - cursorPos.X);
			data[cursorPos.Y].erase(cursorPos.X, data[cursorPos.Y].size() - cursorPos.X);
			data.insert(data.begin() + cursorPos.Y + 1, toMoveBelow);
			cursorPos.Y += 1;
			cursorPos.X = 0;
		}
	}
	adjustCursorsRelativeToCursor(index);
	cursors[index].setPosition(cursorPos);
	cursors[index].setOffset(cursorPos.X);
	return cursorPos;
}

COORD Document::write(const int index, const std::string& text) {
	COORD cursorPos{-1, -1};
	for (const auto letter : text) {
		cursorPos = write(index, letter);
	}
	return cursorPos;
}

COORD Document::erase(const int index) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	auto cursorPos = cursors[index].position();
	if (cursorPos.Y > data.size()) {
		return cursorPos;
	}
	if (cursorPos.X > 0) {
		data[cursorPos.Y].erase(data[cursorPos.Y].begin() + cursorPos.X - 1, data[cursorPos.Y].begin() + cursorPos.X);
		cursorPos.X -= 1;
	}
	else {
		if (cursorPos.Y <= 0) {
			return cursorPos;
		}
		std::string toMoveUpper = data[cursorPos.Y];
		data[cursorPos.Y - 1].erase(data[cursorPos.Y - 1].end() - 1, data[cursorPos.Y - 1].end() - 0);
		data.erase(data.begin() + cursorPos.Y, data.begin() + cursorPos.Y + 1);
		cursorPos.Y -= 1;
		cursorPos.X = data[cursorPos.Y].size();
		data[cursorPos.Y] += toMoveUpper;
	}
	adjustCursorsRelativeToCursor(index);
	cursors[index].setPosition(cursorPos);
	cursors[index].setOffset(cursorPos.X);
	return cursorPos;
}

COORD Document::erase(const int index, const int eraseSize) {
	COORD cursorPos{-1, -1};
	for (int i = 0; i < eraseSize; i++) {
		cursorPos = erase(index);
	}
	return cursorPos;
}

COORD Document::moveCursorLeft(const int index) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	auto cursorPos = cursors[index].position();
	if (cursorPos.X == 0 && cursorPos.Y == 0) {
		return cursorPos;
	}
	if (cursorPos.X > 0) {
		cursorPos.X--;
	}
	else {
		cursorPos.Y--;
		cursorPos.X = data[cursorPos.Y].size() - 1;
	}
	cursors[index].setPosition(cursorPos);
	cursors[index].setOffset(cursorPos.X);
	return cursorPos;
}

COORD Document::moveCursorRight(const int index) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	auto cursorPos = cursors[index].position();
	if (cursorPos.Y == data.size() - 1 && cursorPos.X == data[cursorPos.Y].size()) {
		return cursorPos;
	}
	bool endlPresent = !data[cursorPos.Y].empty() && data[cursorPos.Y][data[cursorPos.Y].size() - 1] == '\n';
	if (cursorPos.X < data[cursorPos.Y].size() - endlPresent) {
		cursorPos.X++;
	}
	else {
		cursorPos.Y++;
		cursorPos.X = 0;
	}
	cursors[index].setPosition(cursorPos);
	cursors[index].setOffset(cursorPos.X);
	return cursorPos;
}

COORD Document::moveCursorUp(const int index, const int bufferWidth) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	auto cursorPos = cursors[index].position();
	cursors[index].setOffset(cursors[index].offset() % bufferWidth);
	auto offset = cursors[index].offset();
	if (cursorPos.X >= bufferWidth) {
		cursorPos.X = (cursorPos.X / bufferWidth - 1) * bufferWidth + offset;
	}
	else if (cursorPos.Y > 0) {
		if (cursorPos.X < bufferWidth) {
			cursorPos.Y--;
		}
		int perfectCursorPos = data[cursorPos.Y].size() / bufferWidth * bufferWidth + offset;
		cursorPos.X = (std::min)(perfectCursorPos, (int)data[cursorPos.Y].size() - 1);
	}
	cursors[index].setPosition(cursorPos);
	return cursorPos;
}

COORD Document::moveCursorDown(const int index, const int bufferWidth) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	auto cursorPos = cursors[index].position();
	cursors[index].setOffset(cursors[index].offset() % bufferWidth);
	auto offset = cursors[index].offset();
	if (data[cursorPos.Y].size() > (cursorPos.X / bufferWidth + 1) * bufferWidth) {
		bool endlPresent = !data[cursorPos.Y].empty() && data[cursorPos.Y][data[cursorPos.Y].size() - 1] == '\n';
		cursorPos.X = (std::min)(cursorPos.X + bufferWidth, (int)data[cursorPos.Y].size() - endlPresent);
	}
	else if (cursorPos.Y != data.size() - 1) {
		cursorPos.Y++;
		bool endlPresent = !data[cursorPos.Y].empty() && data[cursorPos.Y][data[cursorPos.Y].size() - 1] == '\n';
		int perfectCursorPos = (data[cursorPos.Y].size() % bufferWidth) / bufferWidth * bufferWidth + offset;
		cursorPos.X = (std::min)(perfectCursorPos, (int)data[cursorPos.Y].size() - endlPresent);
	}
	cursors[index].setPosition(cursorPos);
	return cursorPos;
}

bool Document::isCursorValid(const int cursor) {
	if (cursor < 0 || cursor >= cursors.size()) {
		return false;
	}
	auto pos = cursors[cursor].position();
	if (pos.Y < 0 || pos.Y >= data.size() || pos.X < 0 || pos.X > data[pos.Y].size()) {
		return false;
	}
	return true;
}

bool Document::addCursor() {
	cursors.emplace_back(Cursor());
	return true;
}

bool Document::eraseCursor(const int cursor) {
	if (cursor < 0 || cursor >= cursors.size()) {
		return false;
	}
	cursors.erase(cursors.cbegin() + cursor);
	if (myCursorIdx > cursor) {
		myCursorIdx--;
	}
	return true;
}

bool Document::setCursorPos(const int index, COORD newPos) {
	if (index < 0 || index >= cursors.size() || newPos.Y >= data.size() || newPos.X > data[newPos.Y].size()) {
		return false;
	}
	cursors[index].setPosition(std::move(newPos));
	return true;
}

bool Document::setCursorOffset(const int index, const int newOffset) {
	if (index < 0 || index >= cursors.size() || newOffset > data[cursors[index].position().Y].size()) {
		return false;
	}
	cursors[index].setOffset(newOffset);
	return true;
}

COORD Document::getCursorPos(const int index) const {
	if (index < 0 || index >= cursors.size()) {
		return COORD{-1, -1};
	}
	return cursors[index].position();
}

int Document::getMyCursor() const {
	return myCursorIdx;
}

void Document::setMyCursor(const int index) {
	myCursorIdx = index;
}

std::vector<COORD> Document::getCursorPositions() const {
	std::vector<COORD> positions;
	for (const auto& cursor : cursors) {
		positions.emplace_back(cursor.position());
	}
	return positions;
}

const std::vector<std::string>& Document::get() {
	return data;
}

std::string Document::getLine(const int lineIndex) const {
	if (lineIndex >= data.size()) {
		return "";
	}
	return data[lineIndex];
}

std::string Document::getText() const {
	std::string text;
	for (const auto& line : data) {
		text += line;
	}
	return text;
}

void Document::setText(const std::string& txt) {
	std::vector<std::string> textData;
	int offset = 0;
	int endLinePos = 0;
	while ((endLinePos = txt.find('\n', offset)) != std::string::npos) {
		textData.emplace_back(txt.substr(offset, endLinePos - offset + 1));
		offset = endLinePos + 1;
	}
	textData.emplace_back(txt.substr(offset, txt.size() - offset));
	data = std::move(textData);
	for (auto& cursor : cursors) {
		cursor.setPosition(COORD{ 0, 0 });
		cursor.setOffset(0);
	}
}

std::string Document::getFilename() const {
	return filename;
}

void Document::adjustCursorsRelativeToCursor(const int index) {
	if (index < 0 || index >= cursors.size()) {
		return;
	}
	for (int i = 0; i < cursors.size(); i++) {
		if (i == index || isCursorValid(i)) {
			continue;
		}
		auto cursorPos = cursors[i].position();
		if (cursorPos.Y >= data.size()) {
			cursorPos.Y = data.size() - 1;
		}
		bool endlPresent = !data[cursorPos.Y].empty() && data[cursorPos.Y][data[cursorPos.Y].size() - 1] == '\n';
		cursorPos.X = data[cursorPos.Y].size() - endlPresent;
		cursors[i].setPosition(cursorPos);
		cursors[i].setOffset(cursorPos.X);
	}
}
