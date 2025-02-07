#pragma once
#define _WINSOCKAPI_ 
#include <Winsock2.h>
#include <Windows.h>
#include <memory>
#include "renderer.h"
#include "window_base.h"
#include "keypack.h"

class Terminal {
public:
	Terminal();
	~Terminal();
	KeyPack readChar() const;
	void render(const std::unique_ptr<BaseWindow>& window);
	void clear() const;
	std::string getClipboardData() const;
	bool setClipboardData(const std::string& txt) const;
	bool resizeScreenBufferIfNeeded(const std::unique_ptr<BaseWindow>& window);
private:

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screenInfo{};
	CONSOLE_CURSOR_INFO cursorInfo{};

	Renderer renderer;
};