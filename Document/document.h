#pragma once
#define _WINSOCKAPI_ 

#include <vector>
#include <string>
#include <optional>
#include "text_container.h"
#include "history_manager.h"

struct User {
	Cursor cursor;
	std::optional<Cursor> selectAnchor;
	bool isSelecting() {
		return selectAnchor.has_value();
	}
};

class SyncTester;
class Document {
public:
	friend class SyncTester;
	Document();
	Document(const std::string& text);
	Document(const std::string& text, const int cursors, const int myUserIdx);
	Document(const std::string& text, const int cursors, const int myUserIdx, const history::HistoryManagerOptions& historyManagerOptions);
	Document(Document&&) noexcept;
	Document& operator=(Document&&) noexcept;
	Document(const Document&) = delete;
	Document& operator=(const Document&) = delete;

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
	bool setCursorAnchor(const int index, const COORD newAnchor);
	COORD getCursorPos(const int index) const;
	COORD getEndPos() const;
	std::optional<COORD> getCursorSelectionAnchor(const int index) const;
	int getMyCursor() const;
	int getCursorNum() const;
	char getCharPointedByCursor(const int index) const;
	std::vector<COORD> getCursorPositions() const;

	const std::vector<std::string>& get() const;
	std::string getLine(const int lineIndex) const;
	std::string getText() const;
	std::string getSelectedText() const;
	std::string getFilename() const;

private:
	bool validateUserIdx(const int index) const;
	COORD eraseSelectedText(const int userIdx);

	void adjustCursor(Cursor& cursor);
	void moveAffectedCursors(User& movedUser, COORD& posDiff);
	void moveAffectedCursor(Cursor& cursor, COORD& moveStartPos, COORD& posDiff);
	bool analyzeBackwardMove(User& user, const bool withSelect);
	bool analyzeForwardMove(User& user, const bool withSelect);
	std::string filename = "document.txt";
	std::vector<User> users;

	TextContainer container;
	history::HistoryManager historyManager;
	int myUserIdx;

};