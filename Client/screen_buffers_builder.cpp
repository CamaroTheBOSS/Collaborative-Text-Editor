#include "screen_buffers_builder.h"

ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::showLineNumbers() {
	lineNumbers = true;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::showLeftFramePattern(const std::string& pattern) {
	leftFramePattern = pattern;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::showRightFramePattern(const std::string& pattern) {
	rightFramePattern = pattern;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::showTopFramePattern(const std::string& pattern) {
	topFramePattern = pattern;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::showBottomFramePattern(const std::string& pattern) {
	botFramePattern = pattern;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setTitle(const std::string& newTitle) {
	title = newTitle;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setScrollHisteresis(const int val) {
	scrollHisteresis = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setAbsoluteTop(const int val) {
	leftTopAbs.Y = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setAbsoluteBot(const int val) {
	rightBottomAbs.Y = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setAbsoluteLeft(const int val) {
	leftTopAbs.X = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setAbsoluteRight(const int val) {
	rightBottomAbs.X = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setRelativeTop(const double val) {
	leftTop.Y = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setRelativeBot(const double val) {
	rightBottom.Y = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setRelativeLeft(const double val) {
	leftTop.X = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setRelativeRight(const double val) {
	rightBottom.X = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setConsoleSize(const Pos<int>& newConsoleSize) {
	consoleSize = newConsoleSize;
	return *this;
}

ScrollableScreenBuffer ScrollableScreenBufferBuilder::getResult() const {
	ScrollableScreenBuffer buffer;
	buffer.title = title;
	buffer.scrollHisteresis = scrollHisteresis;
	buffer.showLineNumbers = lineNumbers;
	buffer.botFramePattern = botFramePattern;
	buffer.topFramePattern = topFramePattern;
	buffer.leftFramePattern = leftFramePattern;
	buffer.rightFramePattern = rightFramePattern;
	buffer.consoleSize = consoleSize;
	if (leftTopAbs.X != 0 || leftTopAbs.Y != 0 || rightBottomAbs.X != 0 || rightBottomAbs.Y != 0) {
		buffer.setBufferAbsoluteSize(leftTopAbs.X, leftTopAbs.Y, rightBottomAbs.X, rightBottomAbs.Y);
	}
	else {
		buffer.setBufferSize(leftTop, rightBottom);
	}
	return buffer;
}