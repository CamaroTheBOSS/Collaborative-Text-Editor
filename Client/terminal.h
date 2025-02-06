#pragma once
#define _WINSOCKAPI_ 
#include <Winsock2.h>
#include <Windows.h>
#include "document.h"
#include "renderer.h"
#include "screen_buffers.h"

#define ARROW_UP 1296
#define ARROW_DOWN 1304
#define ARROW_LEFT 1299
#define ARROW_RIGHT 1301

#define CTRL_A 1
#define CTRL_C 3
#define CTRL_V 22
#define CTRL_X 24
#define CTRL_Z 26

#define BACKSPACE 8
#define TABULAR 9
#define ENTER 13
#define ESC 27

struct KeyPack {
	int keyCode;
	bool shiftPressed;
	std::string str() const {
		return "(" + std::to_string(keyCode) + (shiftPressed ? ", shift)" : ")");
	}
};

class Terminal {
public:
	Terminal();
	~Terminal();
	KeyPack readChar() const;
	void render(Document& doc);
	void clear() const;
	unsigned int getDocBufferWidth() const;
	std::string getClipboardData() const;
	bool setClipboardData(const std::string& txt) const;
	bool resizeScreenBufferIfNeeded();
private:

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screenInfo;
	CONSOLE_CURSOR_INFO cursorInfo;

	Renderer renderer;
	ScrollableScreenBuffer docBuffer;
};