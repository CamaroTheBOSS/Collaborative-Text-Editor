#include "document_base.h"
#include "parser.h"
#include "pos_helpers.h"
#include "line_modifier.h"

#define NOMINMAX

BaseDocument::BaseDocument() :
	container(),
	myUserIdx(0) {}

BaseDocument::BaseDocument(const std::string& text) :
	container(text),
	myUserIdx(0) {}

BaseDocument::BaseDocument(const std::string& text, const int nCursors, const int myUserIdx) :
	container(text),
	myUserIdx(myUserIdx) {
}

COORD BaseDocument::write(const int index, const std::string& newText) {
	if (!validateUserIdx(index)) {
		return COORD{ -1, -1 };
	}
	auto parsedLines = Parser::parseTextToVector(newText);
	COORD startPos = users[index].cursor.position();
	COORD endPos = eraseSelectedText(index);
	endPos = container.insert(endPos, parsedLines);
	COORD diffPos = endPos - startPos;
	moveAffectedCursors(users[index], diffPos);
	users[index].cursor.setOffset(endPos.X);
	afterWriteAction(index, startPos, endPos, parsedLines);
	return endPos;
}

COORD BaseDocument::eraseSelectedText(const int index) {
	auto& user = users[index];
	COORD userPos = user.cursor.position();
	if (!user.selectAnchor.has_value()) {
		return userPos;
	}
	std::vector<std::string> erasedText;
	COORD endPos = container.eraseBetween(userPos, user.selectAnchor.value().position(), erasedText);
	COORD startPos = endPos != userPos ? userPos : user.selectAnchor.value().position();
	user.selectAnchor.reset();
	COORD diffPos = endPos - startPos;
	moveAffectedCursors(user, diffPos);
	user.cursor.setPosition(endPos);
	user.cursor.setOffset(endPos.X);
	afterEraseAction(index, startPos, endPos, erasedText);
	return endPos;
}

bool BaseDocument::validateUserIdx(const int index) const {
	return index >= 0 && index < users.size();
}

COORD BaseDocument::erase(const int index, const int eraseSize) {
	if (!validateUserIdx(index)) {
		return COORD{ -1, -1 };
	}
	if (users[index].isSelecting()) {
		return eraseSelectedText(index);
	}
	std::vector<std::string> erasedText;
	COORD startPos = users[index].cursor.position();
	COORD endPos = container.erase(startPos, eraseSize, erasedText);
	COORD diffPos = endPos - startPos;
	moveAffectedCursors(users[index], diffPos);
	users[index].cursor.setOffset(endPos.X);
	afterEraseAction(index, startPos, endPos, erasedText);
	return endPos;
}

bool BaseDocument::analyzeBackwardMove(User& user, const bool withSelect) {
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

COORD BaseDocument::moveCursorLeft(const int index, const bool withSelect) {
	if (!validateUserIdx(index)) {
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
		cursorPos.X = container.getLineSize(cursorPos.Y);
	}
	cursor.setPosition(cursorPos);
	cursor.setOffset(cursorPos.X);
	return cursorPos;
}

bool BaseDocument::analyzeForwardMove(User& user, const bool withSelect) {
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


COORD BaseDocument::moveCursorRight(const int index, const bool withSelect) {
	if (!validateUserIdx(index)) {
		return COORD{ -1, -1 };
	}
	auto& cursor = users[index].cursor;
	if (!analyzeForwardMove(users[index], withSelect)) {
		return cursor.position();
	}
	auto cursorPos = cursor.position();
	if (cursorPos.Y == container.getHeight() - 1 && cursorPos.X == container.getLineSize(cursorPos.Y)) {
		return cursorPos;
	}
	if (cursorPos.X < container.getLineSize(cursorPos.Y)) {
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

COORD BaseDocument::moveCursorUp(const int index, const int bufferWidth, const bool withSelect) {
	if (!validateUserIdx(index)) {
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
		int perfectCursorPos = container.getLineSize(cursorPos.Y) / bufferWidth * bufferWidth + offset;
		cursorPos.X = (std::min)(perfectCursorPos, (int)container.getLineSize(cursorPos.Y));
	}
	cursor.setPosition(cursorPos);
	return cursorPos;
}

COORD BaseDocument::moveCursorDown(const int index, const int bufferWidth, const bool withSelect) {
	if (!validateUserIdx(index)) {
		return COORD{ -1, -1 };
	}
	auto& cursor = users[index].cursor;
	analyzeForwardMove(users[index], withSelect);
	auto cursorPos = cursor.position();
	cursor.setOffset(cursor.offset() % bufferWidth);
	auto offset = cursor.offset();
	if (container.getLineSize(cursorPos.Y) > (cursorPos.X / bufferWidth + 1) * bufferWidth) {
		cursorPos.X = (std::min)(cursorPos.X + bufferWidth, (int)container.getLineSize(cursorPos.Y));
	}
	else if (cursorPos.Y != container.getHeight() - 1) {
		cursorPos.Y++;
		int perfectCursorPos = (container.getLineSize(cursorPos.Y) % bufferWidth) / bufferWidth * bufferWidth + offset;
		cursorPos.X = (std::min)(perfectCursorPos, (int)container.getLineSize(cursorPos.Y));
	}
	cursor.setPosition(cursorPos);
	return cursorPos;
}

COORD BaseDocument::moveTo(const int index, const COORD newPos, const COORD anchor, const bool withSelect) {
	if (!validateUserIdx(index) || newPos.Y >= container.getHeight() || newPos.X > container.getLineSize(newPos.Y)) {
		return COORD{ -1, -1 };
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

bool BaseDocument::addUser() {
	users.emplace_back(User());
	return true;
}

bool BaseDocument::eraseUser(const int index) {
	if (!validateUserIdx(index)) {
		return false;
	}
	users.erase(users.cbegin() + index);
	if (myUserIdx > index) {
		myUserIdx--;
	}
	return true;
}

bool BaseDocument::setCursorPos(const int index, COORD newPos) {
	if (!validateUserIdx(index) || !container.isPosValid(newPos)) {
		return false;
	}
	users[index].cursor.setPosition(newPos);
	users[index].cursor.setOffset(newPos.X);
	return true;
}

bool BaseDocument::setCursorAnchor(const int index, const COORD newAnchor) {
	if (!validateUserIdx(index) || !container.isPosValid(newAnchor)) {
		return false;
	}
	users[index].selectAnchor = newAnchor;
	return true;
}

COORD BaseDocument::getCursorPos(const int index) const {
	if (!validateUserIdx(index)) {
		return COORD{ -1, -1 };
	}
	return users[index].cursor.position();
}

COORD BaseDocument::getEndPos() const {
	return container.getEndPos();
}

std::optional<COORD> BaseDocument::getCursorSelectionAnchor(const int index) const {
	if (!validateUserIdx(index)) {
		return {};
	}
	const auto& anchor = users[index].selectAnchor;
	if (!anchor.has_value()) {
		return {};
	}
	return { anchor.value().position() };
}

int BaseDocument::getMyCursor() const {
	return myUserIdx;
}

int BaseDocument::getCursorNum() const {
	return users.size();
}

char BaseDocument::getCharPointedByCursor(const int index) const {
	if (!validateUserIdx(index)) {
		return ' ';
	}
	return container.getChar(users[index].cursor.position());
}

std::vector<COORD> BaseDocument::getCursorPositions() const {
	std::vector<COORD> positions;
	for (const auto& user : users) {
		positions.emplace_back(user.cursor.position());
	}
	return positions;
}

const std::vector<std::string>& BaseDocument::get() const {
	return container.get();
}

std::string BaseDocument::getLine(const int col) const {
	return container.getLine(col);
}

std::string BaseDocument::getText() const {
	return container.getText();
}

std::string BaseDocument::getSelectedText() const {
	auto& cursor = users[myUserIdx].cursor;
	auto cursorPos = cursor.position();
	auto& anchor = users[myUserIdx].selectAnchor;
	if (!anchor.has_value() || cursorPos == anchor.value().position()) {
		return "";
	}
	return container.getTextBetween(cursorPos, anchor.value().position());
}

std::string BaseDocument::getFilename() const {
	return filename;
}

void BaseDocument::moveAffectedCursors(User& movedUser, COORD& posDiff) {
	auto movedUserCursorPos = movedUser.cursor.position();
	for (auto& user : users) {
		moveAffectedCursor(user.cursor, movedUserCursorPos, posDiff);
		if (user.selectAnchor.has_value()) {
			moveAffectedCursor(user.selectAnchor.value(), movedUserCursorPos, posDiff);
		}
	}
}

void BaseDocument::moveAffectedCursor(Cursor& cursor, COORD& moveStartPos, COORD& posDiff) {
	auto otherCursorPos = cursor.position();
	if (otherCursorPos.Y == moveStartPos.Y && otherCursorPos.X >= moveStartPos.X) {
		cursor.setPosition(otherCursorPos + posDiff);
	}
	else if (otherCursorPos.Y > moveStartPos.Y) {
		cursor.setPosition(otherCursorPos + COORD{ 0, posDiff.Y });
	}
	adjustCursor(cursor);
}

void BaseDocument::adjustCursor(Cursor& cursor) {
	auto newPos = container.validatePos(cursor.position());
	cursor.setPosition(newPos);
	cursor.setOffset(newPos.X);
}
