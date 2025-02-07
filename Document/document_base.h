#pragma once
#define _WINSOCKAPI_ 

#include <vector>
#include <string>
#include <optional>
#include "text_container.h"
#include "cursor.h"

struct User {
	Cursor cursor;
	std::optional<Cursor> selectAnchor;
	bool isSelecting() {
		return selectAnchor.has_value();
	}
};

class SyncTester;
class BaseDocument {
public:
	friend class SyncTester;
	BaseDocument();
	BaseDocument(const BaseDocument& other);
	BaseDocument(BaseDocument&& other) noexcept;
	BaseDocument(const std::string& text);
	BaseDocument(const std::string& text, const int cursors, const int myUserIdx);
	BaseDocument& operator=(const BaseDocument& other);
	BaseDocument& operator=(BaseDocument&& other) noexcept;

	COORD write(const int index, const std::string& text);
	COORD erase(const int index, const int eraseSize);

	COORD moveCursorLeft(const int index, const bool withSelect);
	COORD moveCursorRight(const int index, const bool withSelect);
	COORD moveCursorUp(const int index, const int bufferWidth, const bool withSelect);
	COORD moveCursorDown(const int index, const int bufferWidth, const bool withSelect);
	COORD moveTo(const int index, const COORD newPos, const COORD anchor, const bool withSelect);

	virtual bool addUser();
	virtual bool eraseUser(const int index);
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

protected:
	bool analyzeBackwardMove(User& user, const bool withSelect);
	bool analyzeForwardMove(User& user, const bool withSelect);
	virtual void pushWriteAction(const int index, const COORD& startPos, std::vector<std::string>& text, TextContainer* target) {}
	virtual void pushEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& text, TextContainer* target) {}

	bool validateUserIdx(const int index) const;
	COORD eraseSelectedText(const int userIdx);

	void adjustCursor(Cursor& cursor);
	void moveAffectedCursors(User& movedUser, COORD& posDiff);
	void moveAffectedCursor(Cursor& cursor, COORD& moveStartPos, COORD& posDiff);

	std::string filename = "document.txt";
	std::vector<User> users;
	TextContainer container;
	int myUserIdx;
};