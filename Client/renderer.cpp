#include "renderer.h"
#include "pos_helpers.h"

#include <iostream>
#include <array>

constexpr std::array<int, 8> colors = { 240, 128, 144, 160, 48, 192, 208, 96 };
constexpr int defaultColor = 7;

void Renderer::render(Document& doc, const ScrollableScreenBuffer& buffer) const {
    auto visibleLines = buffer.getTextInBuffer(doc);
    renderText(buffer, visibleLines, buffer.getStartPos(), buffer.getEndPos());

    auto cursors = buffer.getTerminalCursors(doc);
    auto myCursorSelectionAnchor = doc.getCursorSelectionAnchor(doc.getMyCursor());
    if (myCursorSelectionAnchor.has_value()) {
        auto terminalSelectionAnchor = buffer.getTerminalCursorPos(doc, myCursorSelectionAnchor.value());
        renderSelection(visibleLines, buffer, cursors[doc.getMyCursor()].pos, terminalSelectionAnchor, colors[doc.getMyCursor()]);
    }
    
    auto nCursors = (std::min)(cursors.size(), colors.size());
    for (int i = 0; i < nCursors; i++) {
        renderCursor(buffer, cursors[i], colors[i]);
    }
}

void Renderer::renderText(const ScrollableScreenBuffer& buffer, const std::vector<std::string>& linesToRender, const COORD& startPos, const COORD& endPos) const {
    if (!smallerPos(startPos, endPos)) {
        return;
    }
    auto bufferStartPos = buffer.getStartPos();
    auto renderIndexingBase = diffPos(startPos, bufferStartPos);
    if (startPos.Y == endPos.Y) {
        SetConsoleCursorPosition(hConsole, startPos);
        std::cout << linesToRender[renderIndexingBase.Y].substr(renderIndexingBase.X, endPos.X - startPos.X);
        return;
    }
    
    int nLines = endPos.Y - startPos.Y;
    SetConsoleCursorPosition(hConsole, startPos);
    std::cout << linesToRender[renderIndexingBase.Y].substr(renderIndexingBase.X);
    for (int i = 1; i < nLines; i++) {
        SetConsoleCursorPosition(hConsole, COORD{ bufferStartPos.X, static_cast<SHORT>(startPos.Y + i)});
        std::cout << linesToRender[renderIndexingBase.Y + i];
    }
    SetConsoleCursorPosition(hConsole, COORD{ bufferStartPos.X, endPos.Y});
    std::cout << linesToRender[renderIndexingBase.Y + nLines].substr(0, endPos.X - bufferStartPos.X);
}

void Renderer::renderCursor(const ScrollableScreenBuffer& buffer, const RenderCursor& cursor, const int color) const {
    if (!buffer.isVisible(cursor.pos)) {
        return;
    }
    SetConsoleTextAttribute(hConsole, color);
    SetConsoleCursorPosition(hConsole, cursor.pos);
    std::cout << cursor.pointedChar;
    SetConsoleTextAttribute(hConsole, defaultColor);
}

void Renderer::renderSelection(const std::vector<std::string>& linesToRender, const ScrollableScreenBuffer& buffer, const COORD& cursor, const COORD& anchor, const int color) const {
    if (equalPos(cursor, anchor)) {
        return;
    }
    const COORD* smallerOne; const COORD* biggerOne;
    if (smallerPos(cursor, anchor)) {
        smallerOne = &cursor;
        biggerOne = &anchor;
    }
    else {
        smallerOne = &anchor;
        biggerOne = &cursor;
    }
    const COORD startPos = buffer.getStartPos();
    const COORD endPos = buffer.getEndPos();
    auto& start = smallerPos(*smallerOne, startPos) ? startPos : *smallerOne;
    auto& end = smallerPos(*biggerOne, endPos) ? *biggerOne : endPos;

    SetConsoleTextAttribute(hConsole, color);
    SetConsoleCursorPosition(hConsole, start);
    renderText(buffer, linesToRender, start, end);
    SetConsoleTextAttribute(hConsole, defaultColor);
}
