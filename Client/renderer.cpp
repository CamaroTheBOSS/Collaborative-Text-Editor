#include "renderer.h"

#include <iostream>
#include <array>

constexpr std::array<int, 8> colors = { 240, 128, 144, 160, 48, 192, 208, 96 };
constexpr int defaultColor = 7;

void Renderer::render(Document& doc, const ScrollableScreenBuffer& buffer) const {
	renderDocument(doc, buffer);
    auto cursors = buffer.getTerminalCursors(doc);
    auto nCursors = (std::min)(cursors.size(), colors.size());
    for (int i = 0; i < nCursors; i++) {
        renderCursor(doc, buffer, cursors[i], colors[i]);
    }
}

void Renderer::renderDocument(Document& doc, const ScrollableScreenBuffer& buffer) const {
    auto linesToRender = buffer.getTextInBuffer(doc);  
    auto lineStartPos = buffer.getStartPos();
    for (const auto& line : linesToRender) {
        SetConsoleCursorPosition(hConsole, lineStartPos);
        std::cout << line;
        lineStartPos.Y += 1;
    }
}

void Renderer::renderCursor(Document& doc, const ScrollableScreenBuffer& buffer, const Cursor& cursor, const int color) const {
    if (!buffer.isVisible(cursor.pos)) {
        return;
    }
    SetConsoleTextAttribute(hConsole, color);
    SetConsoleCursorPosition(hConsole, cursor.pos);
    std::cout << cursor.pointedChar;
    SetConsoleTextAttribute(hConsole, defaultColor);
}
