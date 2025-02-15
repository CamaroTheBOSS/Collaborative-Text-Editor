#include "window_replace.h"

constexpr msg::OneByteInt version = 1;

ReplaceWindow::ReplaceWindow() :
    BaseWindow(),
    docToReplace(nullptr) {}

ReplaceWindow::ReplaceWindow(const ScrollableScreenBufferBuilder& ssbBuilder, ClientSiteDocument* docToReplace) :
    BaseWindow(ssbBuilder),
    docToReplace(docToReplace) {}

ActionDone ReplaceWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        doc.write(0, std::string(1, key.keyCode));
        return ActionDone::render;
    }
    switch (key.keyCode) {
    case TABULAR:
        doc.write(0, "    ");
        return ActionDone::render;
    case BACKSPACE:
        doc.erase(0, 1);
        return ActionDone::render;
    case ENTER:
        if (docToReplace->getSegments().empty()) {
            return ActionDone::undone;
        }
        client.sendMsg(msg::Type::replace, version, std::string{""}, doc.getText(), docToReplace->getSegments());
        return ActionDone::render;
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
    case CTRL_A:
        doc.setCursorPos(0, doc.getEndPos());
        doc.setCursorAnchor(0, COORD{ 0, 0 });
        return ActionDone::render;
    case CTRL_V:
        doc.write(0, clipboardData);
        return ActionDone::render;
    case CTRL_X:
        doc.erase(0, 1);
        return ActionDone::render;
    }
    return ActionDone::undone;
}
