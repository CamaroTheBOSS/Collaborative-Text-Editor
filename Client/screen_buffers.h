#pragma once
#include "Windows.h"
#include "document.h"

using TextLines = std::vector<std::string>;

struct Cursor {
    Cursor(const COORD& pos, char pointedChar) :
        pos(pos),
        pointedChar(pointedChar) {}
    COORD pos;
    char pointedChar;
};

class ScrollableScreenBuffer {
public:
    ScrollableScreenBuffer();
    ScrollableScreenBuffer(const SMALL_RECT& screen);
    COORD getStartPos() const;
    Cursor getTerminalCursor(Document& doc, const int cursor) const;
    Cursor getMyTerminalCursor(Document& doc) const;
    std::vector<Cursor> getTerminalCursors(Document& doc) const;
    TextLines getTextInBuffer(Document& doc) const;
    void moveHorizontal(const int units);
    void moveVertical(const int units);
    void scrollScreen(const int units);
    bool isVisible(const COORD& coord) const;
    int height() const;
    int width() const;

    int top, bottom, left, right, scroll;
};