#include <algorithm>
#include <assert.h>

#include "document.h"
#include "parser.h"
#include "pos_helpers.h"
#include "line_modifier.h"
#include "action_write.h"
#include "action_erase.h"

#define NOMINMAX

Document::Document():
	container(),
	historyManager() {
	addUser();
}

Document::Document(const std::string& text):
	container(text),
	historyManager() {
	addUser();
}

Document::Document(const std::string& text, const int nCursors, const int myUserIdx) :
	container(text),
	myUserIdx(myUserIdx),
	historyManager() {
	for (int i = 0; i < nCursors; i++) {
		addUser();
	}
}

Document::Document(const std::string& text, const int nCursors, const int myUserIdx, const history::HistoryManagerOptions& historyManagerOptions):
	container(text),
	myUserIdx(myUserIdx),
	historyManager(historyManagerOptions) {
	for (int i = 0; i < nCursors; i++) {
		addUser();
	}
}


Document::Document(Document&& other) noexcept :
	container(std::move(other.container)),
	users(std::move(other.users)),
	filename(std::move(other.filename)),
	myUserIdx(other.myUserIdx),
	historyManager(std::move(other.historyManager)) {}

Document& Document::operator=(Document&& other) noexcept {
	container = std::move(other.container);
	users = std::move(other.users);
	filename = std::move(other.filename);
	myUserIdx = other.myUserIdx;
	historyManager = std::move(other.historyManager);
	return *this;
}

COORD Document::write(const int index, const std::string& newText) {
	if (!validateUserIdx(index)) {
		return COORD{ -1, -1 };
	}
	auto parsedLines = Parser::parseTextToVector(newText);
	COORD startPos = users[index].cursor.position();
	COORD endPos = eraseSelectedText(index);
	endPos = container.insert(endPos, parsedLines);
	COORD diffPos = endPos - startPos;
	moveAffectedCursors(users[index], diffPos);
	adjustCursors();
	users[index].cursor.setOffset(endPos.X);
	historyManager.pushWriteAction(index, startPos, parsedLines, &container);
	return endPos;
}

COORD Document::eraseSelectedText(const int index) {
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
	adjustCursors();
	user.cursor.setPosition(endPos);
	user.cursor.setOffset(endPos.X);
	historyManager.pushEraseAction(index, startPos, endPos, erasedText, &container);
	return endPos;
}

bool Document::validateUserIdx(const int index) const {
	return index >= 0 && index < users.size();
}

COORD Document::erase(const int index, const int eraseSize) {
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
	adjustCursors();
	users[index].cursor.setOffset(endPos.X);
	historyManager.pushEraseAction(index, startPos, endPos, erasedText, &container);
	return endPos;
}

UndoReturn Document::undo(const int index) {
	if (!validateUserIdx(index)) {
		return { ActionType::noop };
	}
	auto ret = historyManager.undo(index);
	if (ret.type == ActionType::noop) {
		return ret;
	}
	users[index].cursor.setPosition(ret.startPos);
	COORD diff = ret.endPos - ret.startPos;
	moveAffectedCursors(users[index], diff);
	adjustCursors();
	return ret;
}

UndoReturn Document::redo(const int index) {
	if (!validateUserIdx(index)) {
		return { ActionType::noop };
	}
	auto ret = historyManager.redo(index);
	users[index].cursor.setPosition(ret.startPos);
	COORD diff = ret.endPos - ret.startPos;
	moveAffectedCursors(users[index], diff);
	adjustCursors();
	return ret;
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

COORD Document::moveCursorUp(const int index, const int bufferWidth, const bool withSelect) {
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

COORD Document::moveCursorDown(const int index, const int bufferWidth, const bool withSelect) {
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

COORD Document::moveTo(const int index, const COORD newPos, const COORD anchor, const bool withSelect) {
	if (!validateUserIdx(index) || newPos.Y >= container.getHeight() || newPos.X > container.getLineSize(newPos.Y)) {
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

bool Document::addUser() {
	users.emplace_back(User());
	historyManager.addHistory();
	return true;
}

bool Document::eraseUser(const int index) {
	if (!validateUserIdx(index)) {
		return false;
	}
	historyManager.removeHistory(index);
	users.erase(users.cbegin() + index);
	if (myUserIdx > index) {
		myUserIdx--;
	}
	return true;
}

bool Document::setCursorPos(const int index, COORD newPos) {
	if (!validateUserIdx(index) || !container.isPosValid(newPos)) {
		return false;
	}
	users[index].cursor.setPosition(newPos);
	users[index].cursor.setOffset(newPos.X);
	return true;
}

bool Document::setCursorAnchor(const int index, const COORD newAnchor) {
	if (!validateUserIdx(index) || !container.isPosValid(newAnchor)) {
		return false;
	}
	users[index].selectAnchor = newAnchor;
	return true;
}

COORD Document::getCursorPos(const int index) const {
	if (!validateUserIdx(index)) {
		return COORD{-1, -1};
	}
	return users[index].cursor.position();
}

COORD Document::getEndPos() const {
	return container.getEndPos();
}

std::optional<COORD> Document::getCursorSelectionAnchor(const int index) const {
	if (!validateUserIdx(index)) {
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

char Document::getCharPointedByCursor(const int index) const {
	if (!validateUserIdx(index)) {
		return ' ';
	}
	return container.getChar(users[index].cursor.position());
}

std::vector<COORD> Document::getCursorPositions() const {
	std::vector<COORD> positions;
	for (const auto& user : users) {
		positions.emplace_back(user.cursor.position());
	}
	return positions;
}

const std::vector<std::string>& Document::get() const {
	return container.get();
}

std::string Document::getLine(const int col) const {
	return container.getLine(col);
}

std::string Document::getText() const {
	return container.getText();
}

std::string Document::getSelectedText() const {
	auto& cursor = users[myUserIdx].cursor;
	auto cursorPos = cursor.position();
	auto& anchor = users[myUserIdx].selectAnchor;
	if (!anchor.has_value() || cursorPos == anchor.value().position()) {
		return "";
	}
	return container.getTextBetween(cursorPos, anchor.value().position());
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
	auto newPos = container.validatePos(cursor.position());
	cursor.setPosition(newPos);
	cursor.setOffset(newPos.X);
}
