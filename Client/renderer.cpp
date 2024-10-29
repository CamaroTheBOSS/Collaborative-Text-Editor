#include "renderer.h"

#include <iostream>
#include <array>

constexpr std::array<int, 8> colors = { 240, 128, 144, 160, 48, 192, 208, 96 };
constexpr int defaultColor = 7;

void Renderer::render(Document& doc, const ScreenBuffer& screenBuffer) const {
	renderDocument(doc, screenBuffer);
    auto cursors = doc.getCursorPositions();
    for (int i = 0; i < cursors.size(); i++) {
        renderCursor(doc, cursors[i], screenBuffer, colors[i]);
    }
}

void Renderer::renderDocument(Document& doc, const ScreenBuffer& screenBuffer) const {
    auto docCursor = doc.getCursorPos(doc.getMyCursor());
    auto terminalCursor = synchronizeCursor(doc, docCursor, screenBuffer);
    auto linesToRender = getTextToRender(doc, screenBuffer);
    SetConsoleCursorPosition(hConsole, COORD{ 0, screenBuffer.srWindow.Top });
    for (const auto& line : linesToRender) {
        std::cout << line;
    }
}

void Renderer::renderCursor(Document& doc, const COORD& docCursor, const ScreenBuffer& screenBuffer, const int color) const {
    const auto& data = doc.get();
    const auto tPos = synchronizeCursor(doc, docCursor, screenBuffer);
    if (tPos.Y < screenBuffer.srWindow.Top || tPos.Y > screenBuffer.srWindow.Bottom) {
        return;
    }
    SetConsoleTextAttribute(hConsole, color);
    SetConsoleCursorPosition(hConsole, tPos);
    if (docCursor.Y < data.size() && docCursor.X < data[docCursor.Y].size() && data[docCursor.Y][docCursor.X] != '\n') {
        std::cout << data[docCursor.Y][docCursor.X];
    }
    else {
        std::cout << ' ';
    }
    SetConsoleTextAttribute(hConsole, defaultColor);
}

TextLines Renderer::getTextToRender(Document& doc, const ScreenBuffer& screenBuffer) const {
    int tLineCounter = 0;
    TextLines textLines;
    for (const auto& line : doc.get()) {
        int head = 0;
        int tail = (std::min)((int)line.size(), (int)screenBuffer.dwSize.X);
        while (head < tail && tLineCounter <= screenBuffer.srWindow.Bottom) {
            if (tLineCounter < screenBuffer.srWindow.Top) {
                head += screenBuffer.dwSize.X;
                tail = (std::min)((int)line.size(), tail + (int)screenBuffer.dwSize.X);
                tLineCounter++;
                continue;
            }
            std::string tLine = line.substr(head, tail - head);
            if (!tLine.empty() && tLine[tLine.size() - 1] == '\n') {
                tLine[tLine.size() - 1] = ' ';
            }           
            int spaceCount = screenBuffer.dwSize.X - tLine.size();
            tLine += std::string(spaceCount, ' ') + "\n";
            textLines.emplace_back(std::move(tLine));
            head += screenBuffer.dwSize.X;
            tail = (std::min)((int)line.size(), tail + (int)screenBuffer.dwSize.X);
            tLineCounter++;
        }
    }
    int bufferHeight = screenBuffer.srWindow.Bottom - screenBuffer.srWindow.Top + 1;
    for (int i = 0; i < bufferHeight - textLines.size(); i++) {
        std::string emptyLine = std::string(screenBuffer.dwSize.X, ' ') + "\n";
        textLines.emplace_back(std::move(emptyLine));
    }
    if (!textLines.empty()) {
        auto& lastLine = textLines[textLines.size() - 1];
        lastLine.erase(lastLine.size() - 1, 1);
    }
    return textLines;
}

COORD Renderer::synchronizeCursor(Document& doc, const COORD& docCursor, const ScreenBuffer& screenBuffer) const {
    const auto& data = doc.get();
    COORD terminalCursor{ 0, 0 };
    for (int i = 0; i <= docCursor.Y; i++) {
        if (i >= data.size() || data[i].empty()) {
            continue;
        }
        bool endlPresent = data[i][data[i].size() - 1] == '\n' && i != docCursor.Y;
        int base = i != docCursor.Y ? data[i].size() : docCursor.X;
        terminalCursor.Y += base / screenBuffer.dwSize.X + endlPresent;
        if (data[i].size() == screenBuffer.dwSize.X && endlPresent) {
            terminalCursor.Y--;
        }
    }
    terminalCursor.X = docCursor.X % screenBuffer.dwSize.X;
    return terminalCursor;
}
