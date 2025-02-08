#include "window_text_editor.h"

constexpr msg::OneByteInt version = 1;

TextEditorWindow::TextEditorWindow() :
    BaseWindow() {}

TextEditorWindow::TextEditorWindow(const Pos<double>& leftTop, const Pos<double>& rightBottom, const Pos<int>& consoleSize) :
    BaseWindow(leftTop, rightBottom, consoleSize) {}

ActionDone TextEditorWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        client.sendMsg(msg::Type::write, version, std::string{""}, std::string(1, key.keyCode));
        return ActionDone::done;
    }
    bool actionDone = false;
    switch (key.keyCode) {
    case ENTER:
        actionDone = client.sendMsg(msg::Type::write, version, std::string{""}, std::string{'\n'});
        break;
    case TABULAR:
        actionDone = client.sendMsg(msg::Type::write, version, std::string{""}, std::string{"    "});
        break;
    case BACKSPACE:
        actionDone = client.sendMsg(msg::Type::erase, version, std::string{""}, static_cast<unsigned int>(1));
        break;
    case ARROW_LEFT:
        actionDone = client.sendMsg(msg::Type::moveHorizontal, version, std::string{""}, msg::MoveSide::left, key.shiftPressed);
        break;
    case ARROW_RIGHT:
        actionDone = client.sendMsg(msg::Type::moveHorizontal, version, std::string{""}, msg::MoveSide::right, key.shiftPressed);
        break;
    case ARROW_UP:
        actionDone = client.sendMsg(msg::Type::moveVertical, version, std::string{""}, msg::MoveSide::up, getDocBufferWidth(), key.shiftPressed);
        break;
    case ARROW_DOWN:
        actionDone = client.sendMsg(msg::Type::moveVertical, version, std::string{""}, msg::MoveSide::down, getDocBufferWidth(), key.shiftPressed);
        break;
    case CTRL_ARROW_DOWN:
        return ActionDone::down;
    case CTRL_ARROW_UP:
        return ActionDone::up;
    case CTRL_A:
        actionDone = client.sendMsg(msg::Type::selectAll, version, std::string{""});
        break;
    case CTRL_V:
        actionDone = client.sendMsg(msg::Type::write, version, std::string{""}, clipboardData);
        break;
    case CTRL_X:
        actionDone = client.sendMsg(msg::Type::erase, version, std::string{""}, static_cast<unsigned int>(1));
        break;
    case CTRL_Z:
        if (key.shiftPressed) {
            actionDone = client.sendMsg(msg::Type::redo, version, std::string{""});
        }
        else {
            actionDone = client.sendMsg(msg::Type::undo, version, std::string{""});
        }
        break;
    }
    return actionDone ? ActionDone::done : ActionDone::undone;
}
