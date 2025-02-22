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

template <typename T>
struct Pos {
    T X{ 0 };
    T Y{ 0 };
};
struct Frame;
class ScrollableScreenBuffer {
public:
    friend class ScrollableScreenBufferBuilder;
    ScrollableScreenBuffer() = default;
    COORD getStartPos() const;
    COORD getEndPos() const;

    COORD getTerminalCursorPos(const ClientSiteDocument& doc, const COORD& docCursor) const;
    RenderCursor getTerminalCursor(const ClientSiteDocument& doc, const int cursor) const;
    RenderCursor getMyTerminalCursor(const ClientSiteDocument& doc) const;
    std::pair<std::vector<std::pair<COORD, COORD>>, int> getSegmentsTerminalCursorPos(const ClientSiteDocument& doc) const;
    std::vector<RenderCursor> getTerminalCursors(const ClientSiteDocument& doc) const;
    std::pair<ScrollableScreenBuffer, TextLines> getLineNumbersText() const;
    TextLines getTextInBuffer(const ClientSiteDocument& doc) const;

    void moveHorizontal(const int units);
    void moveVertical(const int units);
    void scrollToCursor(const RenderCursor& cursor);
    bool isVisible(const COORD& coord) const;
    bool fitInConsole() const;
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
    std::vector<Frame> getFrames() const;

private:
    void setBufferAbsoluteSizeNoValidation(const int newLeft, const int newTop, const int newRight, const int newBottom);
    void scrollScreen(const int units);
    Pos<double>& validatePos(Pos<double>& pos);
    int validateAbsolutePosX(int X);
    int validateAbsolutePosY(int Y);

    Frame getLeftFrame() const;
    Frame getRightFrame() const;
    Frame getTopFrame() const;
    Frame getBottomFrame() const;
    TextLines getHorizontalFrame(const std::string& pattern) const;

    // Absolute positions
    int top = 0;
    int bottom = 0;
    int left = 0;
    int right = 0;
    int scroll = 0;
    int scrollHisteresis = 0;

    // Console size in pixels e.g. 512x256
    Pos<int> consoleSize;

    // Relative positions to console size with values between <0, 1> 
    Pos<double> leftTop; 
    Pos<double> rightBottom;
    Pos<double> center;

    // Buffer format
    bool showLineNumbers = false;
    std::string leftFramePattern;
    std::string rightFramePattern;
    std::string topFramePattern;
    std::string botFramePattern;
    std::string title;
};

struct Frame {
    ScrollableScreenBuffer buffer;
    TextLines text;
};