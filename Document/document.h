#pragma once
#define _WINSOCKAPI_ 

#include <vector>
#include <string>
#include <optional>
#include "user_history.h"

struct User {
	Cursor cursor;
	std::optional<Cursor> selectAnchor;
	UserHistory history{ std::chrono::milliseconds{ 1000 }, 2000 };
	bool isSelecting() {
		return selectAnchor.has_value();
	}
	User() = default;
	User(const User& other) = delete;
	User& operator=(const User& other) = delete;
	User(User&& other) noexcept :
		cursor(std::move(other.cursor)),
		selectAnchor(std::move(other.selectAnchor)),
		history(std::move(other.history)) {}
	User& operator=(User&& other) noexcept {
		cursor = std::move(other.cursor);
		selectAnchor = std::move(other.selectAnchor);
		history = std::move(other.history);
		return *this;
	}
};

class SyncTester;
class Document {
public:
	friend class SyncTester;
	friend class EraseAction;
	friend class WriteAction;
	Document();
	Document(const std::string& text);
	Document(const std::string& text, const int cursors, const int myUserIdx);

	COORD write(const int index, const std::string& text);
	COORD erase(const int index, const int eraseSize);
	UndoReturn undo(const int index);
	UndoReturn redo(const int index);
	
	COORD moveCursorLeft(const int index, const bool withSelect);
	COORD moveCursorRight(const int index, const bool withSelect);
	COORD moveCursorUp(const int index, const int bufferWidth, const bool withSelect);
	COORD moveCursorDown(const int index, const int bufferWidth, const bool withSelect);
	COORD moveTo(const int index, const COORD newPos, const COORD anchor, const bool withSelec);

	bool addUser();
	bool eraseUser(const int index);
	bool setCursorPos(const int index, const COORD newPos);
	bool setCursorOffset(const int index, const int newOffset);
	bool setCursorAnchor(const int index, const COORD newAnchor);
	COORD getCursorPos(const int index) const;
	COORD getEndPos() const;
	std::optional<COORD> getCursorSelectionAnchor(const int index) const;
	int getMyCursor() const;
	int getCursorNum() const;
	char getCharPointedByCursor(const int index) const;
	std::vector<COORD> getCursorPositions() const;

	const std::vector<std::string>& get();
	std::string getLine(const int lineIndex) const;
	std::string getText() const;
	std::string getSelectedText() const;
	std::string getFilename() const;

private:
	void affectHistories(const int userIdx, const Action& newAction);
	COORD insertText(COORD pos, const std::vector<std::string>& parsedLines);
	std::string& addNewLine(const int col, const std::string_view initText);

	void pushAction(const int userIdx, ActionPtr action);
	COORD eraseText(COORD pos, int eraseSize, std::vector<std::string>& erasedText);
	std::pair<int, std::string> eraseLine(const int col);
	COORD eraseSelectedText(const int userIdx);
	COORD eraseTextBetween(const COORD& cursorPos1, const COORD& cursorPos2, std::vector<std::string>& erasedText);

	bool isCursorValid(Cursor& cursor);
	void adjustCursors();
	void adjustCursor(Cursor& cursor);
	void moveAffectedCursors(User& movedUser, COORD& posDiff);
	void moveAffectedCursor(Cursor& cursor, COORD& moveStartPos, COORD& posDiff);
	bool analyzeBackwardMove(User& user, const bool withSelect);
	bool analyzeForwardMove(User& user, const bool withSelect);
	std::string filename = "document.txt";
	std::vector<User> users;

	std::vector<std::string> data;
	int myUserIdx;

};