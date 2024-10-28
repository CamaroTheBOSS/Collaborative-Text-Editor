#include <conio.h>
#include <iostream>
#include <array>

#include "terminal.h"

constexpr std::array<int, 8> colors = { 240, 128, 144, 160, 48, 192, 208, 96 };
constexpr int defaultColor = 7;

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
    if (!GetConsoleScreenBufferInfo(hConsole, &screenData)) {
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
	return screenInfo.dwSize.X;
}

void Terminal::render(Document& doc) {
    std::vector<COORD> correctCursorPositions = syncCursors(doc);
    adjustBufferPos();
    std::string toPrint = getTextToRender(doc);
    SetConsoleCursorPosition(hConsole, COORD{ 0, screenData.srWindow.Top });
    std::cout << toPrint;
    renderCursors(doc, correctCursorPositions);
}

std::string Terminal::getTextToRender(Document& doc) const {
    int tLineCounter = 0;
    std::string toPrint;
    for (const auto& line : doc.get()) {
        int head = 0;
        int tail = (std::min)((int)line.size(), (int)screenData.dwSize.X);
        while (head < tail && tLineCounter <= screenData.srWindow.Bottom) {
            if (tLineCounter < screenData.srWindow.Top) {
                head += screenData.dwSize.X;
                tail = (std::min)((int)line.size(), tail + (int)screenData.dwSize.X);
                tLineCounter++;
                continue;
            }
            std::string tLine = line.substr(head, tail - head);
            bool endlPresent = !tLine.empty() && tLine[tLine.size() - 1] == '\n';
            int spaceCount = ceil((float)tLine.size() / (float)screenData.dwSize.X) * screenData.dwSize.X - tLine.size();
            tLine.insert(tLine.size() - endlPresent, std::string(spaceCount, ' '));
            tLine[tLine.size() - 1] = ' ';
            toPrint += tLine;
            head += screenData.dwSize.X;
            tail = (std::min)((int)line.size(), tail + (int)screenData.dwSize.X);
            tLineCounter++;
        }
    }
    int bufferCapacity = screenData.dwSize.X * (screenData.srWindow.Bottom - screenData.srWindow.Top + 1);
    int nSpaces = bufferCapacity - toPrint.size();
    if (nSpaces > 0) {
        toPrint += std::string(nSpaces, ' ');
    }
    return toPrint;
}

void Terminal::adjustBufferPos() {
    int topDiff = (screenData.srWindow.Top + 5) - relevantCursorPos.Y;
    int bottomDiff = relevantCursorPos.Y - (screenData.srWindow.Bottom - 5);
    int height = screenData.srWindow.Bottom - screenData.srWindow.Top;
    if (topDiff > 0) {
        screenData.srWindow.Top = (std::max)(screenData.srWindow.Top - topDiff, 0);
        screenData.srWindow.Bottom = screenData.srWindow.Top + height;
    }
    else if (bottomDiff > 0) {
        screenData.srWindow.Top += bottomDiff;
        screenData.srWindow.Bottom += bottomDiff;
    }
    SetConsoleWindowInfo(hConsole, true, &screenData.srWindow);
}

void Terminal::renderCursors(Document& doc, std::vector<COORD>& terminalCursorPositions) {
    const auto& data = doc.get();
    auto documentCursorPositions = doc.getCursorPositions();
    for (int i = 0; i < terminalCursorPositions.size(); i++) {
        auto& tPos = terminalCursorPositions[i];
        auto& dPos = documentCursorPositions[i];
        if (tPos.Y < screenData.srWindow.Top || tPos.Y > screenData.srWindow.Bottom || i >= colors.size()) {
            continue;
        }
        SetConsoleTextAttribute(hConsole, colors[i]);
        SetConsoleCursorPosition(hConsole, tPos);
        if (dPos.Y < data.size() && dPos.X < data[dPos.Y].size() && data[dPos.Y][dPos.X] != '\n') {
            std::cout << data[dPos.Y][dPos.X];
        }
        else {
            std::cout << ' ';
        }
    }
    SetConsoleCursorPosition(hConsole, relevantCursorPos);
    SetConsoleTextAttribute(hConsole, defaultColor);
}

std::vector<COORD> Terminal::syncCursors(Document& doc) {
    CONSOLE_SCREEN_BUFFER_INFO cursorInfo;
    if (!GetConsoleScreenBufferInfo(hConsole, &cursorInfo)) {
        return { COORD{0, 0} };
    }
    const auto& data = doc.get();
    std::vector<COORD> docCursors = doc.getCursorPositions();
    std::vector<COORD> terminalCursors(docCursors.size(), COORD{ 0, 0 });
    for (int c = 0; c < docCursors.size(); c++) {
        for (int i = 0; i <= docCursors[c].Y; i++) {
            if (i >= data.size() || data[i].empty()) {
                continue;
            }
            bool endlPresent = data[i][data[i].size() - 1] == '\n' && i != docCursors[c].Y;
            int base = i != docCursors[c].Y ? data[i].size() : docCursors[c].X;
            terminalCursors[c].Y += base / cursorInfo.dwSize.X + endlPresent;
        }
        terminalCursors[c].X = docCursors[c].X % cursorInfo.dwSize.X;
    }
    relevantCursorPos = terminalCursors[doc.getMyCursor()];
    return terminalCursors;
}