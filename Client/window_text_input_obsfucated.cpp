#include "window_text_input_obsfucated.h"

constexpr msg::OneByteInt version = 1;

ObsfucatedTextInputWindow::ObsfucatedTextInputWindow(const ScrollableScreenBufferBuilder& ssbBuilder, TextInputHandler onSubmitHandler, TextInputHandler onModifyHandler, TextInputHandler onDeleteHandler) :
    TextInputWindow(ssbBuilder, onSubmitHandler, onModifyHandler, onDeleteHandler),
    nonObsfucatedDoc("", 1, 0) {}

Event ObsfucatedTextInputWindow::onDelete() {
    return onDeleteHandler(*this, nonObsfucatedDoc);
}

Event ObsfucatedTextInputWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        doc.write(0, "*");
        nonObsfucatedDoc.write(0, std::string(1, key.keyCode));
        return onModifyHandler(*this, nonObsfucatedDoc);
    }
    switch (key.keyCode) {
    case CTRL_X:
    case BACKSPACE:
        doc.erase(0, 1);
        nonObsfucatedDoc.erase(0, 1);
        return onModifyHandler(*this, nonObsfucatedDoc);
    case ENTER:
        return onSubmitHandler(*this, nonObsfucatedDoc);
    case ARROW_LEFT:
        doc.moveCursorLeft(0, key.shiftPressed);
        nonObsfucatedDoc.moveCursorLeft(0, key.shiftPressed);
        return Event{};
    case ARROW_RIGHT:
        doc.moveCursorRight(0, key.shiftPressed);
        nonObsfucatedDoc.moveCursorRight(0, key.shiftPressed);
        return Event{};
    case ARROW_UP:
        doc.moveCursorUp(0, getDocBufferWidth(), key.shiftPressed);
        nonObsfucatedDoc.moveCursorUp(0, getDocBufferWidth(), key.shiftPressed);
        return Event{};
    case ARROW_DOWN:
        doc.moveCursorDown(0, getDocBufferWidth(), key.shiftPressed);
        nonObsfucatedDoc.moveCursorDown(0, getDocBufferWidth(), key.shiftPressed);
        return Event{};
    case CTRL_A:
        doc.setCursorPos(0, doc.getEndPos());
        doc.setCursorAnchor(0, COORD{ 0, 0 });
        nonObsfucatedDoc.setCursorPos(0, nonObsfucatedDoc.getEndPos());
        nonObsfucatedDoc.setCursorAnchor(0, COORD{ 0, 0 });
        return Event{};
    case CTRL_V:
        doc.write(0, std::string(clipboardData.size(), '*'));
        nonObsfucatedDoc.write(0, clipboardData);
        return onModifyHandler(*this, nonObsfucatedDoc);
    }
    return Event{};
}