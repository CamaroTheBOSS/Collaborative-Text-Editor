#pragma once
#include <string>
#include "screen_buffers.h"

class ScrollableScreenBufferBuilder {
public:
    ScrollableScreenBufferBuilder& showLineNumbers();
    ScrollableScreenBufferBuilder& showLeftFramePattern(const std::string& pattern);
    ScrollableScreenBufferBuilder& showRightFramePattern(const std::string& pattern);
    ScrollableScreenBufferBuilder& showTopFramePattern(const std::string& pattern);
    ScrollableScreenBufferBuilder& showBottomFramePattern(const std::string& pattern);
    ScrollableScreenBufferBuilder& setTitle(const std::string& newTitle);
    ScrollableScreenBufferBuilder& setScrollHisteresis(const int val);
    ScrollableScreenBufferBuilder& setAbsoluteTop(const int val);
    ScrollableScreenBufferBuilder& setAbsoluteBot(const int val);
    ScrollableScreenBufferBuilder& setAbsoluteLeft(const int val);
    ScrollableScreenBufferBuilder& setAbsoluteRight(const int val);
    ScrollableScreenBufferBuilder& setRelativeTop(const double val);
    ScrollableScreenBufferBuilder& setRelativeBot(const double val);
    ScrollableScreenBufferBuilder& setRelativeLeft(const double val);
    ScrollableScreenBufferBuilder& setRelativeRight(const double val);
    ScrollableScreenBufferBuilder& setConsoleSize(const Pos<int>& newConsoleSize);
    ScrollableScreenBuffer getResult() const;
private:
    // Absolute positions
    Pos<int> leftTopAbs;
    Pos<int> rightBottomAbs;

    // Relative positions to console size with values between <0, 1> 
    Pos<double> leftTop;
    Pos<double> rightBottom;

    // Console size in pixels e.g. 512x256
    Pos<int> consoleSize;

    bool lineNumbers = false;
    std::string leftFramePattern;
    std::string rightFramePattern;
    std::string topFramePattern;
    std::string botFramePattern;
    std::string title;

    int scrollHisteresis = 2;
};