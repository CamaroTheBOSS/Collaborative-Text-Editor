#pragma once
#include "Windows.h"
#include "client_document.h"

using TextLines = std::vector<std::string>;
template <typename T> 
struct Pos {
    T X{ 0 };
    T Y{ 0 };
};

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
    ScrollableScreenBuffer() = default;
    ScrollableScreenBuffer(const Pos<double>& leftTop, const Pos<double>& rightBottom, const Pos<int>& consoleSize);
    ScrollableScreenBuffer(const int newLeft, const int newTop, const int newRight, const int newBottom, const Pos<int>& consoleSize);
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
    void setNewConsoleSize(const Pos<int>& newConsoleSize);
    void setBufferSize(Pos<double> newLeftTop, Pos<double> newRightBottom);
    void setBufferAbsoluteSize(const int newLeft, const int newTop, const int newRight, const int newBottom);
    int getLeft() const;
    int getRight() const;
    int getTop() const;
    int getBottom() const;
    Pos<double> getCenter() const;

private:
    void scrollScreen(const int units);
    Pos<double>& validatePos(Pos<double>& pos);
    int validateAbsolutePosX(int X);
    int validateAbsolutePosY(int Y);

    // Absolute positions
    int top = 0;
    int bottom = 0;
    int left = 0;
    int right = 0;
    int scroll = 0;

    // Console size in pixels e.g. 512x256
    Pos<int> consoleSize;

    // Relative positions to console size with values between <0, 1> 
    Pos<double> leftTop; 
    Pos<double> rightBottom;
    Pos<double> center;

    bool showLineNumbers = true;
    const std::string lineNumberSuffix = "|";
};