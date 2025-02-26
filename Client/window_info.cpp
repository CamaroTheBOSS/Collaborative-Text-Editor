#include "window_info.h"

constexpr msg::OneByteInt version = 1;

InfoWindow::InfoWindow(const ScrollableScreenBufferBuilder& ssbBuilder, const std::string& msg) :
    BaseWindow(ssbBuilder),
    winName(ssbBuilder.getTitle()) {
    doc.write(0, msg);
    doc.setCursorPos(0, COORD{ 0, 0 });
}

InfoWindow::InfoWindow(const ScrollableScreenBufferBuilder& ssbBuilder, const std::string&& msg) :
    BaseWindow(ssbBuilder),
    winName(ssbBuilder.getTitle()) {
    doc.write(0, msg);
    doc.setCursorPos(0, COORD{ 0, 0 });
}

Event InfoWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
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

std::string InfoWindow::name() const {
    return winName;
}
