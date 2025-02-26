#include "window_text_input.h"

constexpr msg::OneByteInt version = 1;

TextInputWindow::TextInputHandler TextInputWindow::funcNoop() {
    return [](const TextInputWindow& win, const ClientSiteDocument& doc) {
        return Event{};
    };
}

TextInputWindow::TextInputWindow(const ScrollableScreenBufferBuilder& ssbBuilder, TextInputHandler onSubmitHandler, TextInputHandler onModifyHandler, TextInputHandler onDeleteHandler) :
    BaseWindow(ssbBuilder),
    winName(ssbBuilder.getTitle()),
    onSubmitHandler(onSubmitHandler),
    onModifyHandler(onModifyHandler),
    onDeleteHandler(onDeleteHandler) {}

Event TextInputWindow::onDelete() {
    return onDeleteHandler(*this, doc);
}

Event TextInputWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        doc.write(0, std::string(1, key.keyCode));
        return onModifyHandler(*this, doc);
    }
    switch (key.keyCode) {
    case TABULAR:
        doc.write(0, "    ");
        return onModifyHandler(*this, doc);
    case CTRL_X:
    case BACKSPACE:
        doc.erase(0, 1);
        return onModifyHandler(*this, doc);
    case ENTER:
        return onSubmitHandler(*this, doc);
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
        return onModifyHandler(*this, doc);
    }
    return Event{};
}

std::string TextInputWindow::name() const {
    return winName;
}