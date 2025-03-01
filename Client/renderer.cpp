#include "renderer.h"
#include "pos_helpers.h"

#include <iostream>
#include <array>

constexpr std::array<int, 8> colors = { 240, 128, 144, 160, 48, 192, 208, 96 };
constexpr int foundSegmentsColor = 31;

void Renderer::addToCanvas(Canvas& canvas, const BaseWindow& window) {
    auto& buffer = window.getBuffer();
    auto& doc = window.getDoc();
    auto isActive = window.isActive();
    return addToCanvas(canvas, buffer, doc, isActive);
}

void Renderer::addToCanvas(Canvas& canvas, const ScrollableScreenBuffer& buffer, const ClientSiteDocument& doc, const bool isActive) {
    auto visibleLines = buffer.getTextInBuffer(doc);
    auto frames = buffer.getFrames();
    // Render basic text + frame if applicable
    addTextToCanvas(canvas, buffer, visibleLines, buffer.getStartPos(), buffer.getEndPos());
    for (const auto& frame : frames) {
        if (!frame.text.empty() && frame.buffer.fitInConsole()) {
            addTextToCanvas(canvas, frame.buffer, frame.text, frame.buffer.getStartPos(), frame.buffer.getEndPos());
        }
    }
    // Render found segments if applicable
    int myCursorIdx = doc.getMyCursor();
    auto [terminalSegments, chosenIndex] = buffer.getSegmentsTerminalCursorPos(doc);
    for (int i = 0; i < terminalSegments.size(); i++) {
        int color = i == chosenIndex ? colors[myCursorIdx] : foundSegmentsColor;
        addSelectionToCanvas(canvas, visibleLines, buffer, terminalSegments[i].first, terminalSegments[i].second, color);
    }

    // Render selection if applicable
    auto cursors = buffer.getTerminalCursors(doc);
    if (isActive) {
        auto myCursorSelectionAnchor = doc.getCursorSelectionAnchor(doc.getMyCursor());
        if (myCursorSelectionAnchor.has_value()) {
            auto terminalSelectionAnchor = buffer.getTerminalCursorPos(doc, myCursorSelectionAnchor.value());
            addSelectionToCanvas(canvas, visibleLines, buffer, cursors[doc.getMyCursor()].pos, terminalSelectionAnchor, colors[doc.getMyCursor()]);
        }
    }

    // Render cursors if applicable
    auto nCursors = (std::min)(cursors.size(), colors.size());
    for (int i = 0; i < nCursors; i++) {
        if (i == myCursorIdx && !isActive) {
            continue;
        }
        addCursorToCanvas(canvas, buffer, cursors[i], colors[i]);
    }
}

void Renderer::addTextToCanvas(Canvas& canvas, const ScrollableScreenBuffer& buffer, const std::vector<std::string>& linesToRender, const COORD& startPos, const COORD& endPos, const int color) {
    if (startPos >= endPos || linesToRender.empty()) {
        return;
    }
    auto bufferStartPos = buffer.getStartPos();
    auto renderIndexingBase = startPos - bufferStartPos;
    if (startPos.Y == endPos.Y) {
        canvas.setCursorPosition(startPos);
        canvas.write(linesToRender[renderIndexingBase.Y].substr(renderIndexingBase.X, endPos.X - startPos.X), color);
        return;
    }
    
    int nLines = endPos.Y - startPos.Y;
    canvas.setCursorPosition(startPos);
    canvas.write(linesToRender[renderIndexingBase.Y].substr(renderIndexingBase.X), color);
    for (int i = 1; i < nLines; i++) {
        canvas.setCursorPosition(COORD{ bufferStartPos.X, static_cast<SHORT>(startPos.Y + i) });
        canvas.write(linesToRender[renderIndexingBase.Y + i], color);
    }
    canvas.setCursorPosition(COORD{ bufferStartPos.X, endPos.Y });
    canvas.write(linesToRender[renderIndexingBase.Y + nLines].substr(0, endPos.X - bufferStartPos.X), color);
}

void Renderer::addCursorToCanvas(Canvas& canvas, const ScrollableScreenBuffer& buffer, const RenderCursor& cursor, const int color) {
    if (!buffer.isVisible(cursor.pos)) {
        return;
    }
    canvas.setCursorPosition(cursor.pos);
    canvas.write(std::string{cursor.pointedChar}, color);
}

void Renderer::addSelectionToCanvas(Canvas& canvas, const std::vector<std::string>& linesToRender, const ScrollableScreenBuffer& buffer, const COORD& cursor, const COORD& anchor, const int color) {
    if (cursor == anchor) {
        return;
    }
    const COORD* smallerOne; const COORD* biggerOne;
    if (cursor < anchor) {
        smallerOne = &cursor;
        biggerOne = &anchor;
    }
    else {
        smallerOne = &anchor;
        biggerOne = &cursor;
    }
    const COORD startPos = buffer.getStartPos();
    const COORD endPos = buffer.getEndPos();
    auto& start = *smallerOne < startPos ? startPos : *smallerOne;
    auto& end = *biggerOne < endPos ? *biggerOne : endPos;

    canvas.setCursorPosition(start);
    addTextToCanvas(canvas, buffer, linesToRender, start, end, color);
}
