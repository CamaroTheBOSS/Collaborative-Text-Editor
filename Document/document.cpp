#include <algorithm>
#include "document.h"
#include "parser.h"
#include "pos_helpers.h"
#include "line_modifier.h"
#include "action_write.h"
#include "action_erase.h"

#define NOMINMAX

Document::Document():
	data({""}),
	myUserIdx(0) {
	addUser();
	data.reserve(1024);
}

Document::Document(const std::string& text):
	data({""}) {
	addUser();
	data.reserve(1024);
	auto parsedLines = Parser::parseTextToVector(text);
	insertText(COORD{ 0, 0 }, parsedLines);
}

Document::Document(const std::string& text, const int nCursors, const int myUserIdx) :
	data({""}),
	myUserIdx(myUserIdx) {
	for (int i = 0; i < nCursors; i++) {
		users.emplace_back(User());
	}
	data.reserve(1024);
	auto parsedLines = Parser::parseTextToVector(text);
	insertText(COORD{ 0, 0 }, parsedLines);
}

COORD Document::write(const int index, const std::string& newText) {
	if (index < 0 || index >= users.size()) {
		return COORD{ -1, -1 };
	}
	auto parsedLines = Parser::parseTextToVector(newText);
	COORD startPos = users[index].cursor.position();
	COORD endPos = eraseSelectedText(users[index]);
	endPos = insertText(endPos, parsedLines);
	COORD diffPos = endPos - startPos;
	moveAffectedCursors(users[index], diffPos);
	adjustCursors();
	users[index].cursor.setOffset(endPos.X);
	ActionPtr action = std::make_unique<WriteAction>(startPos, parsedLines);
	pushAction(users[index], std::move(action));
	return endPos;
}

COORD Document::eraseSelectedText(User& user) {
	COORD userPos = user.cursor.position();
	if (!user.selectAnchor.has_value()) {
		return userPos;
	}
	std::vector<std::string> erasedText;
	COORD endPos = eraseTextBetween(userPos, user.selectAnchor.value().position(), erasedText);
	COORD startPos = endPos != userPos ? userPos : user.selectAnchor.value().position();
	user.selectAnchor.reset();
	COORD diffPos = endPos - startPos;
	moveAffectedCursors(user, diffPos);
	adjustCursors();
	user.cursor.setPosition(endPos);
	user.cursor.setOffset(endPos.X);
	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, erasedText);
	pushAction(user, std::move(action));
	return endPos;
}

COORD Document::insertText(COORD pos, const std::vector<std::string>& parsedLines) {
	if (parsedLines.size() == 1) {
		pos.X = LineModifier::insert(data[pos.Y], pos.X, parsedLines[0]);
		return pos;
	}

	std::string toMoveDown = LineModifier::cut(data[pos.Y], pos.X);
	LineModifier::append(data[pos.Y], parsedLines[0]);
	for (int i = 1; i < parsedLines.size(); i++) {
		addNewLine(pos.Y + i, parsedLines[i]);
	}
	pos.Y += parsedLines.size() - 1;
	pos.X = data[pos.Y].size();
	data[pos.Y].append(toMoveDown);
	return pos;
}

std::string& Document::addNewLine(const int col, const std::string_view initText) {
	data.insert(data.cbegin() + col, std::string{ initText });
	return data[col];
}

void Document::pushAction(User& user, ActionPtr action) {
	affectHistories(*action);
	user.history.push(action);
}

void Document::affectHistories(const Action& newAction) {
	for (auto& other : users) {
		other.history.affect(newAction);
	}
}

COORD Document::eraseTextBetween(const COORD& cursor1, const COORD& cursor2, std::vector<std::string>& erasedText) {
	auto [smaller, bigger] = getAscendingOrder(cursor1, cursor2);
	if (smaller->Y == bigger->Y) {
		auto [newX, line] = LineModifier::erase(data[smaller->Y], bigger->X, bigger->X - smaller->X);
		erasedText.emplace_back(std::move(line));
		return *smaller;
	} else {
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

COORD Document::eraseText(COORD pos, int eraseSize, std::vector<std::string>& erasedText) {
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

std::pair<int, std::string> Document::eraseLine(const int col) {
	int size = data[col].size() + 1;
	auto line = data[col];
	data.erase(data.cbegin() + col);
	return { size, std::move(line) };
}

COORD Document::erase(const int index, const int eraseSize) {
	if (index < 0 || index >= users.size()) {
		return COORD{ -1, -1 };
	}
	if (users[index].isSelecting()) {
		return eraseSelectedText(users[index]);
	}
	std::vector<std::string> erasedText;
	COORD startPos = users[index].cursor.position();
	COORD endPos = eraseText(startPos, eraseSize, erasedText);
	COORD diffPos = endPos - startPos;
	moveAffectedCursors(users[index], diffPos);
	adjustCursors();
	users[index].cursor.setOffset(endPos.X);
	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, erasedText);
	pushAction(users[index], std::move(action));
	return endPos;
}

UndoReturn Document::undo(const int index) {
	if (index < 0 || index >= users.size()) {
		return { ActionType::noop };
	}
	auto action = users[index].history.undo();
	if (!action.has_value()) {
		return { ActionType::noop };
	}
	auto [performedAction, ret] = action.value()->undo(index, *this);
	COORD diff = ret.endPos - ret.startPos;
	moveAffectedCursors(users[index], diff);
	adjustCursors();
	affectHistories(*performedAction);
	users[index].history.pushToRedo(performedAction);
	return ret;
}

UndoReturn Document::redo(const int index) {
	if (index < 0 || index >= users.size()) {
		return { ActionType::noop };
	}
	auto action = users[index].history.redo();
	if (!action.has_value()) {
		return { ActionType::noop };
	}
	auto [performedAction, ret] = action.value()->undo(index, *this);
	COORD diff = ret.endPos - ret.startPos;
	moveAffectedCursors(users[index], diff);
	adjustCursors();
	affectHistories(*performedAction);
	users[index].history.pushToUndo(performedAction);
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
		cursorPos.X = data[cursorPos.Y].size();
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
	if (cursorPos.X < data[cursorPos.Y].size()) {
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
		cursorPos.X = (std::min)(perfectCursorPos, (int)data[cursorPos.Y].size());
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
		cursorPos.X = (std::min)(cursorPos.X + bufferWidth, (int)data[cursorPos.Y].size());
	}
	else if (cursorPos.Y != data.size() - 1) {
		cursorPos.Y++;
		int perfectCursorPos = (data[cursorPos.Y].size() % bufferWidth) / bufferWidth * bufferWidth + offset;
		cursorPos.X = (std::min)(perfectCursorPos, (int)data[cursorPos.Y].size());
	}
	cursor.setPosition(cursorPos);
	return cursorPos;
}

COORD Document::moveTo(const int index, const COORD newPos, const COORD anchor, const bool withSelect) {
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
	users[index].cursor.setPosition(newPos);
	users[index].cursor.setOffset(newPos.X);
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

COORD Document::getEndPos() const {
	return COORD{ static_cast<SHORT>(data[data.size() - 1].size()), static_cast<SHORT>(data.size() - 1) };
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
	return Parser::parseVectorToText(data);
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
	std::string text = data[smaller->Y].substr(smaller->X, data[smaller->Y].size() - smaller->X) + "\n";
	for (int row = smaller->Y + 1; row < bigger->Y; row++ ) {
		text += data[row] + "\n";
	}
	text += data[bigger->Y].substr(0, bigger->X);
	return text;
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
