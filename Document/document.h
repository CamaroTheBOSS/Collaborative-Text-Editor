#pragma once
#define _WINSOCKAPI_ 

#include <vector>
#include <string>
#include <optional>
#include "cursor.h"

struct User {
	Cursor cursor;
	std::optional<Cursor> selectAnchor;
	bool isSelecting() {
		return selectAnchor.has_value();
	}
};



class Document {
public:
	Document();
	Document(const std::string& text);
	Document(const std::string& text, const int cursors, const int myUserIdx);

	COORD write(const int index, const std::string& text);
	COORD erase(const int index, const int eraseSize);
	
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
	void setText(const std::string& txt);
	std::string getFilename() const;

private:
	COORD insertText(COORD pos, const std::vector<std::string_view>& parsedLines);
	std::string& addNewLine(const int col, const std::string_view initText);
	std::vector<std::string_view> parseText(const std::string& text) const;

	COORD eraseText(COORD pos, int eraseSize);
	int eraseLine(const int col);
	COORD eraseSelectedText(User& user);
	COORD eraseTextBetween(const COORD& cursorPos1, const COORD& cursorPos2);

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