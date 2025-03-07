#pragma once
#define _WINSOCKAPI_ 
#include <Winsock2.h>
#include <Windows.h>
#include <memory>
#include "window_base.h"
#include "keypack.h"
#include "canvas.h"

class Terminal {
public:
	Terminal();
	~Terminal();
	KeyPack readChar() const;
	void render(const std::vector<std::unique_ptr<BaseWindow>>& windows);
	void clear() const;
	std::string getClipboardData() const;
	bool setClipboardData(const std::string& txt) const;
	bool resizeScreenBufferIfNeeded();
	COORD getScreenSize() const;
private:
	Canvas canvas{ COORD{0, 0} };
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screenInfo{};
	CONSOLE_CURSOR_INFO cursorInfo{};
	COORD screenSize{ 0, 0 };
};