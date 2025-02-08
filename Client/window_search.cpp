#include "window_search.h"

constexpr msg::OneByteInt version = 1;

SearchWindow::SearchWindow() :
    BaseWindow() {}

SearchWindow::SearchWindow(const Pos<double>& leftTop, const Pos<double>& rightBottom, const Pos<int>& consoleSize) :
    BaseWindow(leftTop, rightBottom, consoleSize) {}

ActionDone SearchWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        doc.write(0, std::string(1, key.keyCode));
        return ActionDone::done;
    }
    switch (key.keyCode) {
    case TABULAR:
        doc.write(0, "    ");
        return ActionDone::done;
    case BACKSPACE:
        doc.erase(0, 1);
        return ActionDone::done;
    case ARROW_LEFT:
        doc.moveCursorLeft(0, key.shiftPressed);
        return ActionDone::done;
    case ARROW_RIGHT:
        doc.moveCursorRight(0, key.shiftPressed);
        return ActionDone::done;
    case ARROW_UP:
        doc.moveCursorUp(0, getDocBufferWidth(), key.shiftPressed);
        return ActionDone::done;
    case ARROW_DOWN:
        doc.moveCursorDown(0, getDocBufferWidth(), key.shiftPressed);
        return ActionDone::done;
    case CTRL_ARROW_DOWN:
        return ActionDone::down;
    case CTRL_ARROW_UP:
        return ActionDone::up;
    case CTRL_A:
        doc.setCursorPos(0, doc.getEndPos());
        doc.setCursorAnchor(0, COORD{ 0, 0 });
        return ActionDone::done;
    case CTRL_V:
        doc.write(0, clipboardData);
        return ActionDone::done;
    case CTRL_X:
        doc.erase(0, 1);
        return ActionDone::done;
    }
    return ActionDone::undone;
}
