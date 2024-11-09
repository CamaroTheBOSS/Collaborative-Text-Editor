#pragma once
#include <WinSock2.h>
#include <string>
#include <vector>

class TextContainer {
public:
	class Cursor {
	public:
		Cursor();
		Cursor(const COORD& pos, const int offset, const char label);
		void shiftTo(const COORD& newPos, const TextContainer& doc);
		bool isValid(const TextContainer& doc) const;
		void updateLabel(const TextContainer& doc);
		COORD moveLeft(const TextContainer& doc);
		COORD moveRight(const TextContainer& doc);
		COORD moveUp(const TextContainer& doc, const int width);
		COORD moveDown(const TextContainer& doc, const int width);
		COORD moveBy(const COORD& val, const TextContainer& doc);
		bool operator==(const Cursor& other) const;

		COORD pos;
		int offset;
		char label;
	};

	class Row {
	public:
		Row();
		Row(const std::string& str);

		std::string line;
		bool endl;
		int renderHeight;
	};

	TextContainer();
	TextContainer(const std::string& txt, std::vector<Cursor>& cursors, const int myCursorIdx);

	COORD write(Cursor& cursor, const std::string& txt);
	COORD erase(Cursor& cursor, size_t size);

	Cursor& getCursor(const int idx);
	Cursor& getMyCursor();
	void addCursor(Cursor& cursor);
	void eraseCursor(const int idx);

	const std::string& getLine(const int idx) const;
	const std::vector<Row>& get() const;
private:
	COORD insertText(COORD cursorPos, const std::string& txt);
	void addNewLine(const int col);
	COORD eraseText(COORD cursorPos, size_t size);
	size_t eraseLine(const int col);
	void moveAffectedCursorsBy(const COORD& val, const COORD cursorPos);

	std::vector<Row> data;
	std::vector<Cursor> cursors;
	int myCursorIdx;
};