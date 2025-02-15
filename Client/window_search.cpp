#include "window_search.h"
#include "pos_helpers.h"

constexpr msg::OneByteInt version = 1;

SearchWindow::SearchWindow() :
    BaseWindow(),
    docToSearch(nullptr) {}

SearchWindow::SearchWindow(const ScrollableScreenBufferBuilder& ssbBuilder, ClientSiteDocument* docToSearch) :
    BaseWindow(ssbBuilder),
    docToSearch(docToSearch) {}

SearchWindow::~SearchWindow() {
    docToSearch->resetSegments();
}

ActionDone SearchWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        doc.write(0, std::string(1, key.keyCode));
        docToSearch->findSegments(doc.getText());
        return ActionDone::render;
    }
    switch (key.keyCode) {
    case TABULAR:
        doc.write(0, "    ");
        docToSearch->findSegments(doc.getText());
        return ActionDone::render;
    case BACKSPACE:
        doc.erase(0, 1);
        docToSearch->findSegments(doc.getText());
        return ActionDone::render;
    case ENTER:
        return sendGoToNextSegment(client);
    case ARROW_LEFT:
        doc.moveCursorLeft(0, key.shiftPressed);
        return ActionDone::render;
    case ARROW_RIGHT:
        doc.moveCursorRight(0, key.shiftPressed);
        return ActionDone::render;
    case ARROW_UP:
        doc.moveCursorUp(0, getDocBufferWidth(), key.shiftPressed);
        return ActionDone::render;
    case ARROW_DOWN:
        doc.moveCursorDown(0, getDocBufferWidth(), key.shiftPressed);
        return ActionDone::render;
    case CTRL_ARROW_DOWN:
        return ActionDone::down;
    case CTRL_ARROW_UP:
        return ActionDone::up;
    case CTRL_A:
        doc.setCursorPos(0, doc.getEndPos());
        doc.setCursorAnchor(0, COORD{ 0, 0 });
        return ActionDone::render;
    case CTRL_V:
        doc.write(0, clipboardData);
        docToSearch->findSegments(doc.getText());
        return ActionDone::render;
    case CTRL_X:
        doc.erase(0, 1);
        docToSearch->findSegments(doc.getText());
        return ActionDone::render;
    }
    return ActionDone::undone;
}

ActionDone SearchWindow::sendGoToNextSegment(TCPClient& client) {
    COORD pos = docToSearch->getNextSegmentPos();
    if (pos == COORD{-1, -1}) {
        return ActionDone::undone;
    }
    unsigned int X = pos.X;
    unsigned int Y = pos.Y;
    client.sendMsg(msg::Type::moveTo, version, std::string{""}, X, Y);
    return ActionDone::done;
}
