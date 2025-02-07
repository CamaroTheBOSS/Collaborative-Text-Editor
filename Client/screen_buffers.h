#pragma once
#include "Windows.h"
#include "client_document.h"

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
    COORD getTerminalCursorPos(ClientSiteDocument& doc, const COORD& docCursor) const;
    RenderCursor getTerminalCursor(ClientSiteDocument& doc, const int cursor) const;
    RenderCursor getMyTerminalCursor(ClientSiteDocument& doc) const;
    std::vector<RenderCursor> getTerminalCursors(ClientSiteDocument& doc) const;
    std::pair<ScrollableScreenBuffer, TextLines> getLineNumbersText() const;
    TextLines getTextInBuffer(ClientSiteDocument& doc) const;
    void moveHorizontal(const int units);
    void moveVertical(const int units);
    void scrollToCursor(const RenderCursor& cursor);
    bool isVisible(const COORD& coord) const;
    int height() const;
    int width() const;
    bool isShowingLineNumbers() const;

    int top, bottom, left, right, scroll;

private:
    void scrollScreen(const int units);

    bool showLineNumbers = true;
    const std::string lineNumberSuffix = "|";
};