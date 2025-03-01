#include "window_text_input_obsfucated.h"
#include "renderer.h"

constexpr msg::OneByteInt version = 1;

ObsfucatedTextInputWindow::ObsfucatedTextInputWindow(const ScrollableScreenBufferBuilder& ssbBuilder, TextInputHandler onSubmitHandler, TextInputHandler onModifyHandler, TextInputHandler onDeleteHandler) :
    TextInputWindow(ssbBuilder, onSubmitHandler, onModifyHandler, onDeleteHandler),
    obsfucated("", 1, 0) {}

Event ObsfucatedTextInputWindow::onDelete() {
    return onDeleteHandler(*this, doc);
}

Event ObsfucatedTextInputWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        doc.write(0, std::string(1, key.keyCode));
        obsfucated.write(0, "*");
        return onModifyHandler(*this, doc);
    }
    switch (key.keyCode) {
    case CTRL_X:
    case BACKSPACE:
        doc.erase(0, 1);
        obsfucated.erase(0, 1);
        return onModifyHandler(*this, doc);
    case ENTER:
        return onSubmitHandler(*this, doc);
    case ARROW_LEFT:
        doc.moveCursorLeft(0, key.shiftPressed);
        obsfucated.moveCursorLeft(0, key.shiftPressed);
        return Event{};
    case ARROW_RIGHT:
        doc.moveCursorRight(0, key.shiftPressed);
        obsfucated.moveCursorRight(0, key.shiftPressed);
        return Event{};
    case ARROW_UP:
        doc.moveCursorUp(0, getDocBufferWidth(), key.shiftPressed);
        obsfucated.moveCursorUp(0, getDocBufferWidth(), key.shiftPressed);
        return Event{};
    case ARROW_DOWN:
        doc.moveCursorDown(0, getDocBufferWidth(), key.shiftPressed);
        obsfucated.moveCursorDown(0, getDocBufferWidth(), key.shiftPressed);
        return Event{};
    case CTRL_A:
        doc.setCursorPos(0, doc.getEndPos());
        doc.setCursorAnchor(0, COORD{ 0, 0 });
        obsfucated.setCursorPos(0, obsfucated.getEndPos());
        obsfucated.setCursorAnchor(0, COORD{ 0, 0 });
        return Event{};
    case CTRL_V:
        doc.write(0, clipboardData);
        obsfucated.write(0, std::string(clipboardData.size(), '*'));
        return onModifyHandler(*this, doc);
    }
    return Event{};
}

void ObsfucatedTextInputWindow::render(Canvas& canvas) {
    updateScroll();
    Renderer::addToCanvas(canvas, buffer, obsfucated, active);
}