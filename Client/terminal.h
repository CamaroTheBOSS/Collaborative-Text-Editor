#pragma once
#define _WINSOCKAPI_ 
#include <Winsock2.h>
#include <Windows.h>
#include "document.h"
#include "renderer.h"

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
#define ESC 27

class Terminal {
public:
	Terminal();
	int readChar() const;
	void render(Document& doc) const;
	unsigned int getWidth() const;
private:
	ScreenBuffer adjustScreenBuffer(ScreenBuffer& screenBuffer, const COORD& terminalCursor) const;
	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	Renderer renderer;
};