#include <algorithm>
#include "document.h"

#define NOMINMAX

std::pair<const COORD*, const COORD*> getAscendingOrder(const COORD& pos1, const COORD& pos2) {
	if (smallerPos(pos1, pos2)) {
		return { &pos1, &pos2 };
	}
	return { &pos2, &pos1 };
}

bool smallerPos(const COORD& pos1, const COORD& pos2) {
	if (pos1.Y < pos2.Y) {
		return true;
	}
	else if (pos1.Y == pos2.Y && pos1.X < pos2.X) {
		return true;
	}
	return false;
}

bool equalPos(const COORD& pos1, const COORD& pos2) {
	return pos1.X == pos2.X && pos1.Y == pos2.Y;
}

COORD diffPos(const COORD& pos1, const COORD& pos2) {
	return COORD{ static_cast<SHORT>(pos1.X - pos2.X), static_cast<SHORT>(pos1.Y - pos2.Y) };
}

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
	auto& cursor = cursors[index];
	auto cursorPos = cursors[index].position();
	if (cursorPos.Y > data.size()) {
		return cursorPos;
	}
	if (cursor.selectAnchor.has_value()) {
		cursorPos = eraseTextBetween(cursorPos, cursor.selectAnchor.value());
		cursor.selectAnchor.reset();
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
	adjustCursors();
	cursors[index].setPosition(cursorPos);
	cursors[index].setOffset(cursorPos.X);
	return cursorPos;
}

COORD Document::write(const int index, const std::string& text) {
	COORD cursorPos = COORD{ -1, -1 };
	for (const auto letter : text) {
		if (letter == '\r') {
			continue;
		}
		cursorPos = write(index, letter);
	}
	return cursorPos;
}

COORD Document::eraseTextBetween(const COORD& cursor1, const COORD& cursor2) {
	auto [smaller, bigger] = getAscendingOrder(cursor1, cursor2);
	if (smaller->Y == bigger->Y) {
		data[smaller->Y].erase(smaller->X, bigger->X - smaller->X);
		return *smaller;
	} else {
		int sizeToMoveUp = (std::max)((int)data[bigger->Y].size() - bigger->X, 0);
		std::string toMoveUp = data[bigger->Y].substr(bigger->X, sizeToMoveUp);
		data.erase(data.begin() + smaller->Y + 1, data.begin() + bigger->Y + 1);
		data[smaller->Y].erase(smaller->X, data[smaller->Y].size() - smaller->X);
		data[smaller->Y] += toMoveUp;
	}
	return *smaller;
}

COORD Document::erase(const int index) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	auto& cursor = cursors[index];
	auto cursorPos = cursor.position();
	if (cursorPos.Y > data.size()) {
		return cursorPos;
	}

	if (cursor.selectAnchor.has_value()) {
		cursorPos = eraseTextBetween(cursorPos, cursor.selectAnchor.value());
		cursor.selectAnchor.reset();
	} else if (cursorPos.X > 0) {
		data[cursorPos.Y].erase(data[cursorPos.Y].begin() + cursorPos.X - 1, data[cursorPos.Y].begin() + cursorPos.X);
		cursorPos.X -= 1;
	} else {
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
	adjustCursors();
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

bool Document::analyzeBackwardMove(Cursor& cursor, const bool withSelect) {
	if (!withSelect && cursor.selectAnchor.has_value()) {
		if (smallerPos(cursor.selectAnchor.value(), cursor._pos)) {
			cursor.setPosition(cursor.selectAnchor.value());
		}
		cursor.selectAnchor.reset();
		return false;
	}
	if (withSelect && !cursor.selectAnchor.has_value()) {
		cursor.selectAnchor = cursor.position();
	}
	return true;
}

COORD Document::moveCursorLeft(const int index, const bool withSelect) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	if (!analyzeBackwardMove(cursors[index], withSelect)) {
		return cursors[index].position();
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

bool Document::analyzeForwardMove(Cursor& cursor, const bool withSelect) {
	if (!withSelect && cursor.selectAnchor.has_value()) {
		if (smallerPos(cursor._pos, cursor.selectAnchor.value())) {
			cursor.setPosition(cursor.selectAnchor.value());
		}
		cursor.selectAnchor.reset();
		return false;
	}
	if (withSelect && !cursor.selectAnchor.has_value()) {
		cursor.selectAnchor = cursor.position();
	}
	return true;
}


COORD Document::moveCursorRight(const int index, const bool withSelect) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	if (!analyzeForwardMove(cursors[index], withSelect)) {
		return cursors[index].position();
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

COORD Document::moveCursorUp(const int index, const int bufferWidth, const bool withSelect) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	analyzeBackwardMove(cursors[index], withSelect);
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

COORD Document::moveCursorDown(const int index, const int bufferWidth, const bool withSelect) {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	analyzeForwardMove(cursors[index], withSelect);
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

COORD Document::moveTo(const int cursor, const COORD& newPos, const bool withSelect, const COORD& anchor) {
	if (cursor < 0 || cursor >= cursors.size() || newPos.Y >= data.size() || newPos.X > data[newPos.Y].size()) {
		return COORD{-1, -1};
	}
	auto& cursorObj = cursors[cursor];
	if (withSelect && !cursorObj.selectAnchor.has_value()) {
		cursorObj.selectAnchor = anchor;
	}
	else if (!withSelect){
		cursorObj.selectAnchor.reset();
	}
	cursors[cursor].setPosition(newPos);
	return cursorObj.position();
}

bool Document::isCursorValid(const int cursor) {
	if (cursor < 0 || cursor >= cursors.size()) {
		return false;
	}
	auto pos = cursors[cursor].position();
	if (pos.Y < 0 || pos.Y >= data.size()) {
		return false;
	}
	bool endlPresent = !data[pos.Y].empty() && data[pos.Y][data[pos.Y].size() - 1] == '\n';
	if (pos.X < 0 || pos.X > data[pos.Y].size() - endlPresent) {
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

bool Document::setCursorAnchor(const int index, const COORD newAnchor) {
	if (index < 0 || index >= cursors.size() || newAnchor.Y >= data.size() || newAnchor.X > data[newAnchor.Y].size()) {
		return false;
	}
	cursors[index].selectAnchor = newAnchor;
	return true;
}

COORD Document::getCursorPos(const int index) const {
	if (index < 0 || index >= cursors.size()) {
		return COORD{-1, -1};
	}
	return cursors[index].position();
}

std::optional<COORD> Document::getCursorSelectionAnchor(const int index) const {
	if (index < 0 || index >= cursors.size()) {
		return COORD{ -1, -1 };
	}
	return cursors[index].selectAnchor;
}

int Document::getMyCursor() const {
	return myCursorIdx;
}

int Document::getCursorNum() const {
	return cursors.size();
}

char Document::getCharPointedByCursor(const int cursor) const {
	if (cursor < 0 || cursor >= cursors.size()) {
		return ' ';
	}
	auto cursorPos = cursors[cursor].position();
	if (cursorPos.Y >= data.size() || cursorPos.X >= data[cursorPos.Y].size() || data[cursorPos.Y][cursorPos.X] == '\n') {
		return ' ';
	}
	return data[cursorPos.Y][cursorPos.X];
}

void Document::setMyCursor(const int index) {
	if (index < 0 || index >= cursors.size()) {
		return;
	}
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

std::string Document::getSelectedText() const {
	auto& cursor = cursors[myCursorIdx];
	auto cursorPos = cursor.position();
	auto& anchor = cursors[myCursorIdx].selectAnchor;
	if (!anchor.has_value() || equalPos(cursorPos, anchor.value())) {
		return "";
	}

	auto [smaller, bigger] = getAscendingOrder(cursorPos, anchor.value());
	if (smaller->Y == bigger->Y) {
		return data[smaller->Y].substr(smaller->X, bigger->X - smaller->X);
	}
	std::string text = data[smaller->Y].substr(smaller->X, data[smaller->Y].size() - smaller->X);
	for (int row = smaller->Y + 1; row < bigger->Y; row++ ) {
		text += data[row];
	}
	text += data[smaller->Y].substr(0, bigger->X);
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

void Document::adjustCursors() {
	for (int i = 0; i < cursors.size(); i++) {
		if (isCursorValid(i)) {
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
