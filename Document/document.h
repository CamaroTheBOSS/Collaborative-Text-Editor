#pragma once
#define _WINSOCKAPI_ 

#include <vector>
#include <string>
#include <optional>
#include <string_view>
#include <Windows.h>

bool smallerPos(const COORD& pos1, const COORD& pos2);
bool equalPos(const COORD& pos1, const COORD& pos2);
COORD diffPos(const COORD& pos1, const COORD& pos2);

class Document {
public:
	struct Cursor {
		Cursor() :
			_pos(COORD{ 0, 0 }),
			_offset(0) {}
		void setPosition(const COORD newPos) {
			_pos = newPos;
		}
		void setOffset(int newOffset) {
			_offset = newOffset;
		}
		COORD position() const {
			return _pos;
		}
		int offset() const {
			return _offset;
		}

		COORD _pos;
		int _offset;
		std::optional<COORD> selectAnchor;
	};

	Document();
	Document(const std::string& text);
	Document(const std::string& text, const int cursors, const int myCursor);

	COORD write(const int cursor, const char letter);
	COORD write(const int cursor, const std::string& text);
	COORD erase(const int cursor);
	COORD erase(const int cursor, const int eraseSize);
	COORD eraseTextBetween(const COORD& cursorPos1, const COORD& cursorPos2);

	COORD moveCursorLeft(const int cursor, const bool withSelect);
	COORD moveCursorRight(const int cursor, const bool withSelect);
	COORD moveCursorUp(const int cursor, const int bufferWidth, const bool withSelect);
	COORD moveCursorDown(const int cursor, const int bufferWidth, const bool withSelect);
	COORD moveTo(const int cursor, const COORD& newPos, const bool withSelect, const COORD& anchor);

	bool isCursorValid(const int cursor);
	bool addCursor();
	bool eraseCursor(const int cursor);
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
	bool analyzeBackwardMove(Cursor& cursor, const bool withSelect);
	bool analyzeForwardMove(Cursor& cursor, const bool withSelect);
	std::string filename = "document.txt";
	std::vector<Cursor> cursors;
	std::vector<std::string> data;
	int myCursorIdx;

};