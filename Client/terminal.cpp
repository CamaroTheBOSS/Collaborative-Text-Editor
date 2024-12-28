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
        ENABLE_INSERT_MODE |
        ENABLE_MOUSE_INPUT |
        ENABLE_PROCESSED_INPUT |
        ENABLE_QUICK_EDIT_MODE |
        ENABLE_WINDOW_INPUT |
        ENABLE_VIRTUAL_TERMINAL_INPUT
        );
    auto stdInput = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(stdInput, &currMode);
    SetConsoleMode(stdInput, currMode & ~features);

    CONSOLE_CURSOR_INFO cursorInfo;
    if (GetConsoleCursorInfo(hConsole, &cursorInfo)) {
        cursorInfo.bVisible = 0;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }

    CONSOLE_SCREEN_BUFFER_INFO screenInfo;
    GetConsoleScreenBufferInfo(hConsole, &screenInfo);
    COORD size = {
            screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1,
            screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1
    };
    SetConsoleScreenBufferSize(hConsole, size);
    screenInfo.srWindow.Top += 3;
    screenInfo.srWindow.Bottom -= 3;
    screenInfo.srWindow.Right -= 10;
    screenInfo.srWindow.Left += 10;
    docBuffer = ScrollableScreenBuffer{ screenInfo.srWindow };
}

KeyPack Terminal::readChar() const {
	if (_kbhit()) {
        int keyCode = _getch();
        if (keyCode == 0 || keyCode == 224) {
            keyCode += _getch() + 1000;
        }
        bool shiftPressed = ((GetKeyState(VK_SHIFT) & 0x8000) == 0x8000);
        return { keyCode, shiftPressed };
	}
    return { '\0', false };
}

void Terminal::render(Document& doc) {
    auto tCursor = docBuffer.getMyTerminalCursor(doc);
    scrollDocBuffer(tCursor);
    return renderer.render(doc, docBuffer);
}

void Terminal::scrollDocBuffer(const Cursor& tCursor) {
    int topDiff = (docBuffer.top + 2) - tCursor.pos.Y;
    int bottomDiff = tCursor.pos.Y - (docBuffer.bottom - 2);
    if (topDiff > 0) {
        docBuffer.scrollScreen(-topDiff);
    }
    else if (bottomDiff > 0) {
        docBuffer.scrollScreen(bottomDiff);
    }
}

unsigned int Terminal::getDocBufferWidth() const {
    return docBuffer.width();
}
