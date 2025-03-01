#include "screen_buffers_builder.h"

std::string ScrollableScreenBufferBuilder::getTitle() const {
	return title;
}

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
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setAbsoluteWidth(const int val) {
	sizeAbs.X = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setAbsoluteHeight(const int val) {
	sizeAbs.Y = val;
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
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setRelativeWidth(const double val) {
	size.X = val;
	return *this;
}
ScrollableScreenBufferBuilder& ScrollableScreenBufferBuilder::setRelativeHeight(const double val) {
	size.Y = val;
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

	bool absPosSet = leftTopAbs.X != 0 || leftTopAbs.Y != 0 || rightBottomAbs.X != 0 || rightBottomAbs.Y != 0;
	bool absSizeSet = sizeAbs.X && sizeAbs.Y;
	bool relPosSet = leftTop.X != 0 || leftTop.Y != 0 || rightBottom.X != 0 || rightBottom.Y != 0;
	bool relSizeSet = size.X && size.Y;
	if (absPosSet) {
		if (absSizeSet) {
			buffer.setBufferAbsoluteSize(leftTopAbs.X, leftTopAbs.Y, leftTopAbs.X + sizeAbs.X, leftTopAbs.Y + sizeAbs.Y);
		}
		else if (relSizeSet) {
			buffer.setBufferAbsoluteSize(leftTopAbs.X, leftTopAbs.Y, rightBottomAbs.X, rightBottomAbs.Y);
			buffer.setBufferSize(
				Pos<double>{buffer.getRelativeLeft(), buffer.getRelativeTop()}, 
				Pos<double>{buffer.getRelativeLeft() + size.X, buffer.getRelativeTop() + size.Y}
			);
		}
		else {
			buffer.setBufferAbsoluteSize(leftTopAbs.X, leftTopAbs.Y, rightBottomAbs.X, rightBottomAbs.Y);
		}
	}
	else if (relPosSet) {
		if (absSizeSet) {
			buffer.setBufferSize(leftTop, rightBottom);
			buffer.setBufferAbsoluteSize(buffer.getLeft(), buffer.getTop(), buffer.getLeft() + sizeAbs.X, buffer.getTop() + sizeAbs.Y);
		}
		else if (relSizeSet) {
			buffer.setBufferSize(leftTop, Pos<double>{leftTop.X + size.X, leftTop.Y + size.Y});
		}
		else {
			buffer.setBufferSize(leftTop, rightBottom);
		}
	}
	return buffer;
}