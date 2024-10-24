#pragma once
#include <Winsock2.h>
#include "document.h"

#define ARROW_UP 1296
#define ARROW_DOWN 1304
#define ARROW_LEFT 1299
#define ARROW_RIGHT 1301

#define CTRL_A 1
#define CTRL_C 3
#define CTRL_V 22
#define CTRL_Q 17

#define BACKSPACE 8
#define TABULAR 9
#define ENTER 13

class Terminal {
public:
	Terminal();
	int readChar() const;
	void render(Document& doc);
	unsigned int getWidth() const;
private:
	void renderCursors(Document& doc);
	COORD syncCursors(Document& doc);
	void setCursorPos(const COORD& newPos);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD cursorPos{0, 0};
};