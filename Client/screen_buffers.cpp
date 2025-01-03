#include "screen_buffers.h"

ScrollableScreenBuffer::ScrollableScreenBuffer() :
	top(0),
	bottom(0),
	left(0),
	right(0),
	scroll(0) {}

ScrollableScreenBuffer::ScrollableScreenBuffer(const SMALL_RECT& rect) :
	top(rect.Top),
	bottom(rect.Bottom),
	left(rect.Left),
	right(rect.Right),
	scroll(0) {}

void ScrollableScreenBuffer::moveHorizontal(const int units) {
	left += units;
	right += units;
}

void ScrollableScreenBuffer::moveVertical(const int units) {
	top += units;
	bottom += units;
}

void ScrollableScreenBuffer::scrollScreen(const int units) {
	scroll = (std::max)(scroll + units, 0);
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

COORD ScrollableScreenBuffer::getTerminalCursorPos(Document& doc, const COORD& docCursor) const {
	const auto& data = doc.get();
	if (docCursor.X < 0) {
		return COORD{0, 0};
	}

	int screenWidth = width();
	COORD terminalCursor{ 0, 0 };
	for (int i = 0; i <= docCursor.Y; i++) {
		if (i >= data.size() || data[i].empty()) {
			continue;
		}
		bool endlPresent = data[i][data[i].size() - 1] == '\n' && i != docCursor.Y;
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

RenderCursor ScrollableScreenBuffer::getTerminalCursor(Document& doc, const int cursor) const {
	auto terminalCursor = getTerminalCursorPos(doc, doc.getCursorPos(cursor));
	return RenderCursor(terminalCursor, doc.getCharPointedByCursor(cursor), cursor);
}

RenderCursor ScrollableScreenBuffer::getMyTerminalCursor(Document& doc) const {
	return getTerminalCursor(doc, doc.getMyCursor());
}

std::vector<RenderCursor> ScrollableScreenBuffer::getTerminalCursors(Document& doc) const {
	std::vector<RenderCursor> terminalCursors;
	for (int cursor = 0; cursor < doc.getCursorNum(); cursor++) {
		terminalCursors.emplace_back(getTerminalCursor(doc, cursor));
	}
	return terminalCursors;
}

TextLines ScrollableScreenBuffer::getTextInBuffer(Document& doc) const {
	//NOTE i think screenWidth should be screen.width() - 1!!!
	//THE SAME WITH screenHeight!!!!
	int tLineCounter = 0;
	TextLines textLines;
	int screenWidth = width();
	int screenHeight = height();
	int topBorder = scroll;
	int botBorder = scroll + screenHeight;
	for (const auto& line : doc.get()) {
		int head = 0;
		int tail = (std::min)((int)line.size(), screenWidth);
		while (head < tail && tLineCounter <= botBorder) {
			if (tLineCounter < topBorder) {
				head += screenWidth;
				tail = (std::min)((int)line.size(), tail + screenWidth);
				tLineCounter++;
				continue;
			}
			std::string tLine = line.substr(head, tail - head);
			if (!tLine.empty() && tLine[tLine.size() - 1] == '\n') {
				tLine[tLine.size() - 1] = ' ';
			}
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
	return coord.X >= left && coord.X <= right
		&& coord.Y >= top && coord.Y <= bottom;
}