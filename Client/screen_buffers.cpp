#include "screen_buffers.h"

void ScrollableScreenBuffer::moveHorizontal(const int units) {
	setBufferAbsoluteSize(left + units, top, right + units, bottom);
}

void ScrollableScreenBuffer::moveVertical(const int units) {
	setBufferAbsoluteSize(left, top + units, right, bottom + units);
}

void ScrollableScreenBuffer::scrollScreen(const int units) {
	scroll = (std::max)(scroll + units, 0);
}

void ScrollableScreenBuffer::scrollToCursor(const RenderCursor& cursor) {
	int topDiff = (top + scrollHisteresis) - cursor.pos.Y;
	int bottomDiff = cursor.pos.Y - (bottom - scrollHisteresis);
	if (topDiff > 0) {
		scrollScreen(-topDiff);
	}
	else if (bottomDiff > 0) {
		scrollScreen(bottomDiff);
	}
}

bool ScrollableScreenBuffer::isShowingLineNumbers() const {
	return showLineNumbers;
}

int ScrollableScreenBuffer::width() const {
	return right - left;
}

int ScrollableScreenBuffer::height() const {
	return bottom - top;
}

COORD ScrollableScreenBuffer::getStartPos() const {
	return COORD{ static_cast<SHORT>(left), static_cast<SHORT>(top) };
}

COORD ScrollableScreenBuffer::getEndPos() const {
	return COORD{ static_cast<SHORT>(right), static_cast<SHORT>(bottom) };
}

COORD ScrollableScreenBuffer::getTerminalCursorPos(ClientSiteDocument& doc, const COORD& docCursor) const {
	const auto& data = doc.get();
	int screenWidth = width();
	if (docCursor.X < 0 || screenWidth <= 0) {
		return COORD{0, 0};
	}

	COORD terminalCursor{ 0, 0 };
	for (int i = 0; i <= docCursor.Y; i++) {
		if (i >= data.size()) {
			continue;
		}
		bool endlPresent = i != docCursor.Y;
		int base = i != docCursor.Y ? data[i].size() : docCursor.X;
		terminalCursor.Y += base / screenWidth + endlPresent;
		if (data[i].size() == screenWidth && endlPresent) {
			terminalCursor.Y--;
		}
	}
	terminalCursor.Y += top - scroll;
	terminalCursor.X += left + (docCursor.X % screenWidth);
	return terminalCursor;
}

RenderCursor ScrollableScreenBuffer::getTerminalCursor(ClientSiteDocument& doc, const int cursor) const {
	auto terminalCursor = getTerminalCursorPos(doc, doc.getCursorPos(cursor));
	return RenderCursor(terminalCursor, doc.getCharPointedByCursor(cursor), cursor);
}

RenderCursor ScrollableScreenBuffer::getMyTerminalCursor(ClientSiteDocument& doc) const {
	return getTerminalCursor(doc, doc.getMyCursor());
}

std::vector<RenderCursor> ScrollableScreenBuffer::getTerminalCursors(ClientSiteDocument& doc) const {
	std::vector<RenderCursor> terminalCursors;
	for (int cursor = 0; cursor < doc.getCursorNum(); cursor++) {
		terminalCursors.emplace_back(getTerminalCursor(doc, cursor));
	}
	return terminalCursors;
}

std::pair<ScrollableScreenBuffer, TextLines> ScrollableScreenBuffer::getLineNumbersText() const {
	TextLines textLines;
	if (!showLineNumbers) {
		return std::make_pair(ScrollableScreenBuffer(), textLines);
	}
	int screenHeight = height();
	int lastLineIndex = scroll + screenHeight + 1;
	int desiredSize = std::to_string(lastLineIndex).size() + leftFramePattern.size() + 1;
	for (int n = scroll + 1; n <= lastLineIndex; n++) {
		std::string base = std::to_string(n) + leftFramePattern;
		std::string line = std::string(desiredSize - base.size(), ' ') + base;
		textLines.emplace_back(std::move(line));
	}
	ScrollableScreenBuffer buffer = *this;
	buffer.setBufferAbsoluteSize(left - desiredSize, top, left, bottom);
	return std::make_pair(buffer, textLines);
}

TextLines ScrollableScreenBuffer::getTextInBuffer(ClientSiteDocument& doc) const {
	int tLineCounter = 0;
	TextLines textLines;
	int screenWidth = width();
	int screenHeight = height();
	if (screenWidth <= 0 || screenHeight <= 0) {
		return textLines;
	}
	int topBorder = scroll;
	int botBorder = scroll + screenHeight;
	for (const auto& line : doc.get()) {
		int head = 0;
		int tail = (std::min)((int)line.size(), screenWidth);
		while (head <= tail && tLineCounter <= botBorder) {
			if (tLineCounter < topBorder) {
				head += screenWidth;
				tail = (std::min)((int)line.size(), tail + screenWidth);
				tLineCounter++;
				continue;
			}
			std::string tLine = line.substr(head, tail - head);
			int spaceCount = screenWidth - tLine.size();
			tLine += std::string(spaceCount, ' ') + "\n";
			textLines.emplace_back(std::move(tLine));
			head += screenWidth;
			tail = (std::min)((int)line.size(), tail + screenWidth);
			tLineCounter++;
		}
	}

	int textLinesSize = textLines.size();
	for (int i = 0; i <= screenHeight - textLinesSize; i++) {
		std::string emptyLine = std::string(screenWidth, ' ') + "\n";
		textLines.emplace_back(std::move(emptyLine));
	}
	if (!textLines.empty()) {
		auto& lastLine = textLines[textLines.size() - 1];
		lastLine[lastLine.size() - 1] = ' ';
	}
	return textLines;
}

bool ScrollableScreenBuffer::isVisible(const COORD& coord) const {
	return coord.X >= getLeft() && coord.X <= getRight()
		&& coord.Y >= getTop() && coord.Y <= getBottom();
}

bool ScrollableScreenBuffer::fitInConsole() const {
	return left >= 0 && left < consoleSize.X &&
		right >= 0 && right < consoleSize.X &&
		top >= 0 && top < consoleSize.Y &&
		bottom >= 0 && bottom < consoleSize.Y;
}

void ScrollableScreenBuffer::setNewConsoleSize(const Pos<int>& newConsoleSize) {
	consoleSize = newConsoleSize;
	setBufferSize(leftTop, rightBottom);
}

void ScrollableScreenBuffer::setBufferSize(Pos<double> newLeftTop, Pos<double> newRightBottom) {
	leftTop = validatePos(newLeftTop);
	rightBottom = validatePos(newRightBottom);
	center.X = (leftTop.X + rightBottom.X) / 2.;
	center.Y = (leftTop.Y + rightBottom.Y) / 2.;
	left = leftTop.X * consoleSize.X;
	right = rightBottom.X * consoleSize.X;
	top = leftTop.Y * consoleSize.Y;
	bottom = rightBottom.Y * consoleSize.Y;
}

void ScrollableScreenBuffer::setBufferAbsoluteSize(const int newLeft, const int newTop, const int newRight, const int newBottom) {
	left = validateAbsolutePosX(newLeft);
	right = validateAbsolutePosX(newRight);
	top = validateAbsolutePosY(newTop);
	bottom = validateAbsolutePosY(newBottom);
	leftTop = Pos<double>((double)left / (double)consoleSize.X, (double)top / (double)consoleSize.Y);
	rightBottom = Pos<double>((double)right / (double)consoleSize.X, (double)bottom / (double)consoleSize.Y);
	center.X = (leftTop.X + rightBottom.X) / 2.;
	center.Y = (leftTop.Y + rightBottom.Y) / 2.;
}

int ScrollableScreenBuffer::getLeft() const {
	return left;
}

int ScrollableScreenBuffer::getRight() const {
	return right;
}

int ScrollableScreenBuffer::getTop() const {
	return top;
}

int ScrollableScreenBuffer::getBottom() const {
	return bottom;
}

Pos<double> ScrollableScreenBuffer::getCenter() const {
	return center;
}

Pos<double>& ScrollableScreenBuffer::validatePos(Pos<double>& pos) {
	pos.X = (std::max)((std::min)(1., pos.X), 0.);
	pos.Y = (std::max)((std::min)(1., pos.Y), 0.);
	return pos;
}

int ScrollableScreenBuffer::validateAbsolutePosX(int X) {
	return (std::max)((std::min)(consoleSize.X, X), 0);
}

int ScrollableScreenBuffer::validateAbsolutePosY(int Y) {
	return (std::max)((std::min)(consoleSize.Y, Y), 0);
}

Frame ScrollableScreenBuffer::getLeftFrame() const {
	if (!showLineNumbers && !leftFramePattern.empty()) {
		TextLines textLines = TextLines(std::vector<std::string>(height() + 1, leftFramePattern));
		ScrollableScreenBuffer buffer = *this;
		buffer.setBufferAbsoluteSize(left - leftFramePattern.size(), top, left, bottom);
		return { std::move(buffer), std::move(textLines) };
	}
	else if (showLineNumbers) {
		TextLines textLines;
		int screenHeight = height();
		int lastLineIndex = scroll + screenHeight + 1;
		int desiredSize = std::to_string(lastLineIndex).size() + leftFramePattern.size() + 2;
		for (int n = scroll + 1; n <= lastLineIndex; n++) {
			std::string base = std::to_string(n) + leftFramePattern;
			std::string line = std::string(desiredSize - base.size(), ' ') + base;
			textLines.emplace_back(std::move(line));
		}
		ScrollableScreenBuffer buffer = *this;
		buffer.setBufferAbsoluteSize(left - desiredSize, top, left, bottom);
		return { std::move(buffer), std::move(textLines) };
	}
	return {};
}

Frame ScrollableScreenBuffer::getRightFrame() const {
	if (rightFramePattern.empty()) {
		return {};
	}
	TextLines textLines = TextLines(std::vector<std::string>(height() + 1, rightFramePattern));
	ScrollableScreenBuffer buffer = *this;
	buffer.setBufferAbsoluteSize(right, top, right + rightFramePattern.size(), bottom);
	return { std::move(buffer), std::move(textLines) };
}

TextLines ScrollableScreenBuffer::getHorizontalFrame(const std::string& pattern) const {
	if (pattern.empty()) {
		return TextLines();
	}
	int lastLineIndex = scroll + height() + 1;
	int desiredSize = leftFramePattern.size() + width() + rightFramePattern.size();
	std::string line;
	while (line.size() < desiredSize) {
		line += topFramePattern;
	}
	if (line.size() > desiredSize) {
		line.erase(desiredSize, line.size() - desiredSize);
	}
	return { std::move(line) };
}

Frame ScrollableScreenBuffer::getTopFrame() const {
	if (topFramePattern.empty()) {
		return {};
	}
	ScrollableScreenBuffer buffer = *this;
	buffer.setBufferAbsoluteSize(left - leftFramePattern.size(), top - 1, right + rightFramePattern.size(), top - 1);
	return { std::move(buffer), getHorizontalFrame(topFramePattern) };
}

Frame ScrollableScreenBuffer::getBottomFrame() const {
	if (botFramePattern.empty()) {
		return {};
	}
	ScrollableScreenBuffer buffer = *this;
	buffer.setBufferAbsoluteSize(left - leftFramePattern.size(), bottom + 1, right + rightFramePattern.size(), bottom + 1);
	return { std::move(buffer), getHorizontalFrame(botFramePattern) };
}

std::vector<Frame> ScrollableScreenBuffer::getFrames() const {
	return { getLeftFrame(), getTopFrame(), getRightFrame(), getBottomFrame() };
}