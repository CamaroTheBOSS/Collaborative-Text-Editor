#include <conio.h>
#include <iostream>

#include "terminal.h"
#include "logging.h"

using namespace client;

bool screenBuffersEqual(const SMALL_RECT& first, const SMALL_RECT& second) {
    return first.Top == second.Top && first.Bottom == second.Bottom &&
        first.Left == second.Left && first.Right == second.Right;
}

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

    if (GetConsoleCursorInfo(hConsole, &cursorInfo)) {
        cursorInfo.bVisible = 0;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }
}

Terminal::~Terminal() {
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
    SetConsoleMode(stdInput, currMode | features);
    resizeScreenBufferIfNeeded();
}

COORD Terminal::getScreenSize() const {
    return screenSize;
}

bool Terminal::resizeScreenBufferIfNeeded() {
    CONSOLE_SCREEN_BUFFER_INFO newScreenInfo;
    GetConsoleScreenBufferInfo(hConsole, &newScreenInfo);
    if (screenBuffersEqual(screenInfo.srWindow, newScreenInfo.srWindow)) {
        return false;
    }
    COORD size = {
            newScreenInfo.srWindow.Right - newScreenInfo.srWindow.Left + 1,
            newScreenInfo.srWindow.Bottom - newScreenInfo.srWindow.Top + 1
    };
    SetConsoleScreenBufferSize(hConsole, size);
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    screenInfo = std::move(newScreenInfo);
    screenSize = std::move(size);
    clear();
    return true;
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

void Terminal::clear() const {
    //system("cls") breaks winsock2 recv somehow? Need to use approach with printf;
    printf("\033[2J"); // clear the screen          
    printf("\033[1;1H"); // move cursor home
}

void Terminal::render(const std::unique_ptr<BaseWindow>& window) {
    auto& docBuffer = window->getBuffer();
    auto& doc = window->getDoc();
    docBuffer.scrollToCursor(docBuffer.getMyTerminalCursor(doc));
    return renderer.render(doc, docBuffer);
}

std::string Terminal::getClipboardData() const {
    if (!OpenClipboard(nullptr)) {
        logger.logError("Error during opening clipboard for read! Error code: ", GetLastError());
        return "";
    }
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        logger.logError("Error during getting handler for clipboard object! Error code: ", GetLastError());
        return "";
    }
    char* txtPointer = static_cast<char*>(GlobalLock(hData));
    if (txtPointer == nullptr) {
        logger.logError("Error during reading data from clipboard! Error code: ", GetLastError());
        return "";
    }
    std::string txt(txtPointer);
    GlobalUnlock(hData);
    CloseClipboard();
    return txt;
}

bool Terminal::setClipboardData(const std::string& txt) const {
    if (txt.empty()) {
        return false;
    }
    HGLOBAL spaceForData = GlobalAlloc(GMEM_MOVEABLE, txt.size() + 1);
    auto allocMemoryPointer = GlobalLock(spaceForData);
    if (allocMemoryPointer == nullptr) {
        logger.logError("Error during getting handler for writing data to clipboard! Error code: ", GetLastError());
        return false;
    }
    memcpy(allocMemoryPointer, txt.data(), txt.size() + 1);
    GlobalUnlock(spaceForData);

    if (!OpenClipboard(nullptr)) {
        logger.logError("Error during opening clipboard for write! Error code: ", GetLastError());
        return false;
    }
    if (!EmptyClipboard()) {
        logger.logError("Error during emptying clipboard! Error code: ", GetLastError());
        return false;
    }
    HANDLE hData = SetClipboardData(CF_TEXT, spaceForData);
    if (hData == nullptr) {
        logger.logError("Error during setting clipboard data! Error code: ", GetLastError());
        return "";
    }
    CloseClipboard();
    return true;
}
