#pragma once
#define _WINSOCKAPI_ 

#include <vector>
#include <string>
#include <string_view>
#include <Windows.h>

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
	};

	Document();
	Document(const std::string& text);
	Document(const std::string& text, const int cursors, const int myCursor);

	COORD write(const int cursor, const char letter);
	COORD write(const int cursor, const std::string& text);
	COORD erase(const int cursor);
	COORD erase(const int cursor, const int eraseSize);

	COORD moveCursorLeft(const int cursor);
	COORD moveCursorRight(const int cursor);
	COORD moveCursorUp(const int cursor, const int bufferWidth);
	COORD moveCursorDown(const int cursor, const int bufferWidth);

	bool addCursor();
	bool eraseCursor(const int cursor);
	bool setCursorPos(const int cursor, const COORD newPos);
	bool setCursorOffset(const int cursor, const int newOffset);
	COORD getCursorPos(const int cursor) const;
	int getMyCursor() const;
	void setMyCursor(const int cursor);
	std::vector<COORD> getCursorPositions() const;

	const std::vector<std::string>& get();
	std::string getLine(const int lineIndex) const;
	std::string getText() const;
	void setText(const std::string& txt);
	std::string getFilename() const;

private:
	void adjustCursorsRelativeToCursor(const int cursor);
	std::string filename = "document.txt";
	std::vector<Cursor> cursors;
	std::vector<std::string> data;
	int myCursorIdx;
	
};