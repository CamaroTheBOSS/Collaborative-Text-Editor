#include <algorithm>
#include "document.h"
#include "pos_helpers.h"

#define NOMINMAX

Document::Document():
	data({""}),
	users({ User() }),
	myUserIdx(0) {
	data.reserve(1024);
}

Document::Document(const std::string& text):
	data(),
	users({ User() }) {
	data.reserve(text.size());
	setText(text);
}

Document::Document(const std::string& text, const int nCursors, const int myUserIdx) :
	data(),
	myUserIdx(myUserIdx),
	users(nCursors, User()) {
	data.reserve(text.size());
	setText(text);
}

COORD Document::write(const int index, const char letter) {
	if (index < 0 || index >= users.size()) {
		return COORD{ -1, -1 };
	}
	auto& cursor = users[index].cursor;
	auto& anchor = users[index].selectAnchor;
	auto cursorPos = cursor.position();
	if (cursorPos.Y > data.size()) {
		return cursorPos;
	}
	if (anchor.has_value()) {
		cursorPos = eraseTextBetween(cursorPos, anchor.value().position());
		anchor.reset();
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
	auto startPos = cursor.position();
	auto diff = cursorPos - startPos;
	moveAffectedCursors(users[index], diff);
	adjustCursors();
	//cursor.setPosition(cursorPos);
	cursor.setOffset(cursorPos.X);
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
	if (index < 0 || index >= users.size()) {
		return COORD{ -1, -1 };
	}
	auto& cursor = users[index].cursor;
	auto& anchor = users[index].selectAnchor;
	auto cursorPos = cursor.position();
	if (cursorPos.Y > data.size()) {
		return cursorPos;
	}

	if (anchor.has_value()) {
		cursorPos = eraseTextBetween(cursorPos, anchor.value().position());
		anchor.reset();
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
	auto diff = cursorPos - cursor.position();
	moveAffectedCursors(users[index], diff);
	adjustCursors();
	//cursor.setPosition(cursorPos);
	cursor.setOffset(cursorPos.X);
	return cursorPos;
}

COORD Document::erase(const int index, const int eraseSize) {
	COORD cursorPos{-1, -1};
	for (int i = 0; i < eraseSize; i++) {
		cursorPos = erase(index);
	}
	return cursorPos;
}

bool Document::analyzeBackwardMove(User& user, const bool withSelect) {
	auto& cursor = user.cursor;
	auto& anchor = user.selectAnchor;
	if (!withSelect && anchor.has_value()) {
		if (anchor.value().position() < cursor.position()) {
			cursor.setPosition(anchor.value().position());
		}
		anchor.reset();
		return false;
	}
	if (withSelect && !anchor.has_value()) {
		anchor = cursor;
	}
	return true;
}

COORD Document::moveCursorLeft(const int index, const bool withSelect) {
	if (index < 0 || index >= users.size()) {
		return COORD{ -1, -1 };
	}
	auto& cursor = users[index].cursor;
	if (!analyzeBackwardMove(users[index], withSelect)) {
		return cursor.position();
	}
	auto cursorPos = cursor.position();
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
	cursor.setPosition(cursorPos);
	cursor.setOffset(cursorPos.X);
	return cursorPos;
}

bool Document::analyzeForwardMove(User& user, const bool withSelect) {
	auto& cursor = user.cursor;
	auto& anchor = user.selectAnchor;
	if (!withSelect && anchor.has_value()) {
		if (cursor.position() < anchor.value().position()) {
			cursor.setPosition(anchor.value().position());
		}
		anchor.reset();
		return false;
	}
	if (withSelect && !anchor.has_value()) {
		anchor = cursor;
	}
	return true;
}


COORD Document::moveCursorRight(const int index, const bool withSelect) {
	if (index < 0 || index >= users.size()) {
		return COORD{ -1, -1 };
	}
	auto& cursor = users[index].cursor;
	if (!analyzeForwardMove(users[index], withSelect)) {
		return cursor.position();
	}
	auto cursorPos = cursor.position();
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
	cursor.setPosition(cursorPos);
	cursor.setOffset(cursorPos.X);
	return cursorPos;
}

COORD Document::moveCursorUp(const int index, const int bufferWidth, const bool withSelect) {
	if (index < 0 || index >= users.size()) {
		return COORD{ -1, -1 };
	}
	auto& cursor = users[index].cursor;
	analyzeBackwardMove(users[index], withSelect);
	auto cursorPos = cursor.position();
	cursor.setOffset(cursor.offset() % bufferWidth);
	auto offset = cursor.offset();
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
	cursor.setPosition(cursorPos);
	return cursorPos;
}

COORD Document::moveCursorDown(const int index, const int bufferWidth, const bool withSelect) {
	if (index < 0 || index >= users.size()) {
		return COORD{ -1, -1 };
	}
	auto& cursor = users[index].cursor;
	analyzeForwardMove(users[index], withSelect);
	auto cursorPos = cursor.position();
	cursor.setOffset(cursor.offset() % bufferWidth);
	auto offset = cursor.offset();
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
	cursor.setPosition(cursorPos);
	return cursorPos;
}

COORD Document::moveTo(const int index, const COORD& newPos, const bool withSelect, const COORD& anchor) {
	if (index < 0 || index >= users.size() || newPos.Y >= data.size() || newPos.X > data[newPos.Y].size()) {
		return COORD{-1, -1};
	}
	auto& cursor = users[index].cursor;
	auto& currAnchor = users[index].selectAnchor;
	if (withSelect) {
		currAnchor = anchor;
	}
	else {
		currAnchor.reset();
	}
	cursor.setPosition(newPos);
	return cursor.position();
}

bool Document::isCursorValid(Cursor& cursor) {
	auto pos = cursor.position();
	if (pos.Y < 0 || pos.Y >= data.size()) {
		return false;
	}
	bool endlPresent = !data[pos.Y].empty() && data[pos.Y][data[pos.Y].size() - 1] == '\n';
	if (pos.X < 0 || pos.X > data[pos.Y].size() - endlPresent) {
		return false;
	}
	return true;
}

bool Document::addUser() {
	users.emplace_back(User());
	return true;
}

bool Document::eraseUser(const int index) {
	if (index < 0 || index >= users.size()) {
		return false;
	}
	users.erase(users.cbegin() + index);
	if (myUserIdx > index) {
		myUserIdx--;
	}
	return true;
}

bool Document::setCursorPos(const int index, COORD newPos) {
	if (index < 0 || index >= users.size() || newPos.Y >= data.size() || newPos.X > data[newPos.Y].size()) {
		return false;
	}
	users[index].cursor.setPosition(std::move(newPos));
	return true;
}

bool Document::setCursorOffset(const int index, const int newOffset) {
	if (index < 0 || index >= users.size() || newOffset > data[users[index].cursor.position().Y].size()) {
		return false;
	}
	users[index].cursor.setOffset(newOffset);
	return true;
}

bool Document::setCursorAnchor(const int index, const COORD newAnchor) {
	if (index < 0 || index >= users.size() || newAnchor.Y >= data.size() || newAnchor.X > data[newAnchor.Y].size()) {
		return false;
	}
	users[index].selectAnchor = newAnchor;
	return true;
}

COORD Document::getCursorPos(const int index) const {
	if (index < 0 || index >= users.size()) {
		return COORD{-1, -1};
	}
	return users[index].cursor.position();
}

std::optional<COORD> Document::getCursorSelectionAnchor(const int index) const {
	if (index < 0 || index >= users.size()) {
		return {};
	}
	const auto& anchor = users[index].selectAnchor;
	if (!anchor.has_value()) {
		return {};
	}
	return { anchor.value().position() };
}

int Document::getMyCursor() const {
	return myUserIdx;
}

int Document::getCursorNum() const {
	return users.size();
}

char Document::getCharPointedByCursor(const int cursor) const {
	if (cursor < 0 || cursor >= users.size()) {
		return ' ';
	}
	auto cursorPos = users[cursor].cursor.position();
	if (cursorPos.Y >= data.size() || cursorPos.X >= data[cursorPos.Y].size() || data[cursorPos.Y][cursorPos.X] == '\n') {
		return ' ';
	}
	return data[cursorPos.Y][cursorPos.X];
}

void Document::setMyCursor(const int index) {
	if (index < 0 || index >= users.size()) {
		return;
	}
	myUserIdx = index;
}

std::vector<COORD> Document::getCursorPositions() const {
	std::vector<COORD> positions;
	for (const auto& user : users) {
		positions.emplace_back(user.cursor.position());
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
	auto& cursor = users[myUserIdx].cursor;
	auto cursorPos = cursor.position();
	auto& anchor = users[myUserIdx].selectAnchor;
	if (!anchor.has_value() || cursorPos == anchor.value().position()) {
		return "";
	}

	auto [smaller, bigger] = getAscendingOrder(cursorPos, anchor.value().position());
	if (smaller->Y == bigger->Y) {
		return data[smaller->Y].substr(smaller->X, bigger->X - smaller->X);
	}
	std::string text = data[smaller->Y].substr(smaller->X, data[smaller->Y].size() - smaller->X);
	for (int row = smaller->Y + 1; row < bigger->Y; row++ ) {
		text += data[row];
	}
	text += data[bigger->Y].substr(0, bigger->X);
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
	for (auto& user : users) {
		user.cursor.setPosition(COORD{ 0, 0 });
		user.cursor.setOffset(0);
	}
}

std::string Document::getFilename() const {
	return filename;
}

void Document::moveAffectedCursors(User& movedUser, COORD& posDiff) {
	auto movedUserCursorPos = movedUser.cursor.position();
	for (auto& user : users) {
		moveAffectedCursor(user.cursor, movedUserCursorPos, posDiff);
		if (user.selectAnchor.has_value()) {
			moveAffectedCursor(user.selectAnchor.value(), movedUserCursorPos, posDiff);
		}
	}
}

void Document::moveAffectedCursor(Cursor& cursor, COORD& moveStartPos, COORD& posDiff) {
	auto otherCursorPos = cursor.position();
	if (otherCursorPos.Y == moveStartPos.Y && otherCursorPos.X >= moveStartPos.X) {
		cursor.setPosition(otherCursorPos + posDiff);
	}
	else if (otherCursorPos.Y > moveStartPos.Y) {
		cursor.setPosition(otherCursorPos + COORD{ 0, posDiff.Y });
	}
}

void Document::adjustCursors() {
	for (auto& user : users) {
		adjustCursor(user.cursor);
		if (user.selectAnchor.has_value()) {
			adjustCursor(user.selectAnchor.value());
		}
	}
}

void Document::adjustCursor(Cursor& cursor) {
	if (isCursorValid(cursor)) {
		return;
	}
	auto cursorPos = cursor.position();
	if (cursorPos.Y >= data.size()) {
		cursorPos.Y = data.size() - 1;
	}
	bool endlPresent = !data[cursorPos.Y].empty() && data[cursorPos.Y][data[cursorPos.Y].size() - 1] == '\n';
	cursorPos.X = data[cursorPos.Y].size() - endlPresent;
	cursor.setPosition(cursorPos);
	cursor.setOffset(cursorPos.X);
}
