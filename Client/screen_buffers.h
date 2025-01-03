#pragma once
#include "Windows.h"
#include "document.h"

using TextLines = std::vector<std::string>;

struct RenderCursor {
    RenderCursor(const COORD& pos, const char pointedChar, const int indexInDoc) :
        pos(pos),
        pointedChar(pointedChar),
        indexInDoc(indexInDoc) {}
    COORD pos;
    char pointedChar;
    int indexInDoc;
};

class ScrollableScreenBuffer {
public:
    ScrollableScreenBuffer();
    ScrollableScreenBuffer(const SMALL_RECT& screen);
    COORD getStartPos() const;
    COORD getEndPos() const;
    COORD getTerminalCursorPos(Document& doc, const COORD& docCursor) const;
    RenderCursor getTerminalCursor(Document& doc, const int cursor) const;
    RenderCursor getMyTerminalCursor(Document& doc) const;
    std::vector<RenderCursor> getTerminalCursors(Document& doc) const;
    TextLines getTextInBuffer(Document& doc) const;
    void moveHorizontal(const int units);
    void moveVertical(const int units);
    void scrollScreen(const int units);
    bool isVisible(const COORD& coord) const;
    int height() const;
    int width() const;

    int top, bottom, left, right, scroll;
};