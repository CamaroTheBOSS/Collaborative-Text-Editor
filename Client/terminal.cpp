#include <conio.h>
#include <iostream>
#include <array>

#include "terminal.h"

constexpr int rightMargin = 5;

Terminal::Terminal() {
    DWORD currMode;
    DWORD features = (
        ENABLE_LINE_INPUT |
        ENABLE_ECHO_INPUT |
        ENABLE_MOUSE_INPUT |
        ENABLE_PROCESSED_INPUT |
        ENABLE_QUICK_EDIT_MODE |
        ENABLE_WINDOW_INPUT |
        ENABLE_PROCESSED_OUTPUT
        );
    auto stdInput = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(stdInput, &currMode);
    SetConsoleMode(stdInput, currMode & ~features);
    system("cls");

    CONSOLE_CURSOR_INFO cursorInfo;
    if (!GetConsoleCursorInfo(hConsole, &cursorInfo)) {
        return;
    }
    cursorInfo.bVisible = 0;
    if (!SetConsoleCursorInfo(hConsole, &cursorInfo)) {
        return;
    }
}

int Terminal::readChar() const {
	if (_kbhit()) {
        int keyCode = _getch();
        if (keyCode == 0 || keyCode == 224) {
            keyCode += _getch() + 1000;
        }
        bool shiftPressed = ((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000);
        return keyCode;
	}
	return '\0';
}

unsigned int Terminal::getWidth() const {
	CONSOLE_SCREEN_BUFFER_INFO screenInfo;
	if (!GetConsoleScreenBufferInfo(hConsole, &screenInfo)) {
		return 0;
	}
	return screenInfo.dwSize.X - rightMargin;
}

void Terminal::render(Document& doc) const {
    ScreenBuffer screenBuffer;
    if (!GetConsoleScreenBufferInfo(hConsole, &screenBuffer)) {
        return;
    }
    screenBuffer.dwSize.X -= rightMargin;
    auto docCursor = doc.getCursorPos(doc.getMyCursor());
    auto terminalCursor = renderer.synchronizeCursor(doc, docCursor, screenBuffer);
    adjustScreenBuffer(screenBuffer, terminalCursor);
    renderer.render(doc, screenBuffer);
}

ScreenBuffer Terminal::adjustScreenBuffer(ScreenBuffer& screenBuffer, const COORD& terminalCursor) const {
    int topDiff = (screenBuffer.srWindow.Top + 5) - terminalCursor.Y;
    int bottomDiff = terminalCursor.Y - (screenBuffer.srWindow.Bottom - 5);
    if (topDiff > 0) {
        int height = screenBuffer.srWindow.Bottom - screenBuffer.srWindow.Top;
        screenBuffer.srWindow.Top = (std::max)(screenBuffer.srWindow.Top - topDiff, 0);
        screenBuffer.srWindow.Bottom = screenBuffer.srWindow.Top + height;
    }
    else if (bottomDiff > 0) {
        screenBuffer.srWindow.Top += bottomDiff;
        screenBuffer.srWindow.Bottom += bottomDiff;
    }
    SetConsoleWindowInfo(hConsole, true, &screenBuffer.srWindow);
    return screenBuffer;
}

