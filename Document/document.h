#pragma once
#define _WINSOCKAPI_ 

#include <vector>
#include <string>
#include <optional>
#include "user_action_history.h"

struct User {
	Cursor cursor;
	std::optional<Cursor> selectAnchor;
	UserActionHistory history{ std::chrono::milliseconds(1000) };
};

class Document {
public:
	Document();
	Document(const std::string& text);
	Document(const std::string& text, const int cursors, const int myUserIdx);

	COORD write(User& user, const char letter, const bool fromAction = false);
	COORD write(const int cursor, const std::string& text, const bool fromAction = false);
	COORD erase(User& user, const bool fromAction = false);
	COORD erase(const int cursor, const int eraseSize, const bool fromAction = false);
	COORD eraseTextBetween(const COORD& cursorPos1, const COORD& cursorPos2);
	ActionPtr undo(const int cursor);
	ActionPtr redo(const int cursor);

	COORD moveCursorLeft(const int cursor, const bool withSelect);
	COORD moveCursorRight(const int cursor, const bool withSelect);
	COORD moveCursorUp(const int cursor, const int bufferWidth, const bool withSelect);
	COORD moveCursorDown(const int cursor, const int bufferWidth, const bool withSelect);
	COORD moveTo(const int cursor, const COORD& newPos, const bool withSelect, const COORD& anchor);

	bool isCursorValid(Cursor& cursor);
	bool addUser();
	bool eraseUser(const int cursor);
	bool setCursorPos(const int cursor, const COORD newPos);
	bool setCursorOffset(const int cursor, const int newOffset);
	bool setCursorAnchor(const int cursor, const COORD newAnchor);
	COORD getCursorPos(const int cursor) const;
	std::optional<COORD> getCursorSelectionAnchor(const int cursor) const;
	int getMyCursor() const;
	int getCursorNum() const;
	char getCharPointedByCursor(const int cursor) const;
	void setMyCursor(const int cursor);
	std::vector<COORD> getCursorPositions() const;

	const std::vector<std::string>& get();
	std::string getLine(const int lineIndex) const;
	std::string getText() const;
	std::string getSelectedText() const;
	void setText(const std::string& txt);
	std::string getFilename() const;

private:
	void adjustCursors();
	void adjustCursor(Cursor& cursor);
	void affectHistory(ActionPtr action, const COORD& diffPos);
	void moveAffectedCursors(User& movedUser, COORD& posDiff);
	void moveAffectedCursor(Cursor& cursor, COORD& moveStartPos, COORD& posDiff);
	bool analyzeBackwardMove(User& user, const bool withSelect);
	bool analyzeForwardMove(User& user, const bool withSelect);
	std::string filename = "document.txt";
	std::vector<User> users;

	std::vector<std::string> data;
	int myUserIdx;

};