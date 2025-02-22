#include "window_help.h"

constexpr msg::OneByteInt version = 1;

HelpWindow::HelpWindow(const ScrollableScreenBufferBuilder& ssbBuilder) :
    BaseWindow(ssbBuilder) {
    doc.write(0,
        "Collaborative Text Editor!\n"
        "Choose 'Create Document' to create document on the server\n"
        "Choose 'Load Document' to connect to existing document\n"
        "F3/CTRL+F - find\n"
        "CTRL+SHIFT+F - find and replace\n"
        "CTRL+Q - open main menu\n"
        "ESC - close last window\n"
        "CTRL+Arrow(up/down/left/right) - change active window\n\n"
        "Credits\n"
        "Main programmer: Kacper Plesiak\n"
        "Main designer: Kacper Plesiak\n"
        "LICENSE: MIT\n"
        "C++ is cool\n"
    );
    doc.setCursorPos(0, COORD{ 0, 0 });
}

Event HelpWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    switch (key.keyCode) {
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
    }
    return Event{};
}
