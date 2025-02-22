#include "window_replace.h"

constexpr msg::OneByteInt version = 1;

ReplaceWindow::ReplaceWindow(const ScrollableScreenBufferBuilder& ssbBuilder) :
    BaseWindow(ssbBuilder) {}

Event ReplaceWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        doc.write(0, std::string(1, key.keyCode));
        return Event{};
    }
    switch (key.keyCode) {
    case TABULAR:
        doc.write(0, "    ");
        return Event{};
    case BACKSPACE:
        doc.erase(0, 1);
        return Event{};
    case ENTER:
        return Event{ windows::text_editor::events::replace, windows::text_editor::name, { doc.getText() } };
    case ARROW_LEFT:
        doc.moveCursorLeft(0, key.shiftPressed);
        return Event{};
    case ARROW_RIGHT:
        doc.moveCursorRight(0, key.shiftPressed);
        return Event{};
    case ARROW_UP:
        doc.moveCursorUp(0, getDocBufferWidth(), key.shiftPressed);
        return Event{};
    case ARROW_DOWN:
        doc.moveCursorDown(0, getDocBufferWidth(), key.shiftPressed);
        return Event{};
    case CTRL_A:
        doc.setCursorPos(0, doc.getEndPos());
        doc.setCursorAnchor(0, COORD{ 0, 0 });
        return Event{};
    case CTRL_V:
        doc.write(0, clipboardData);
        return Event{};
    case CTRL_X:
        doc.erase(0, 1);
        return Event{};
    }
    return Event{};
}
