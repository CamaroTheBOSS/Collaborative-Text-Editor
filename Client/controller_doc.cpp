#include "controller_doc.h"

constexpr msg::OneByteInt version = 1;

DocController::DocController(TCPClient& client, Terminal& terminal) :
    BaseController(client, terminal) {}

bool DocController::processChar(const KeyPack& key) {
    
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        return client.sendMsg(msg::Type::write, version, std::string{""}, std::string(1, key.keyCode));
    }
    switch (key.keyCode) {
    case ENTER:
        return client.sendMsg(msg::Type::write, version, std::string{""}, std::string{'\n'});
    case TABULAR:
        return client.sendMsg(msg::Type::write, version, std::string{""}, std::string{"    "});
    case BACKSPACE:
        return client.sendMsg(msg::Type::erase, version, std::string{""}, static_cast<unsigned int>(1));
    case ARROW_LEFT:
        return client.sendMsg(msg::Type::moveHorizontal, version, std::string{""}, msg::MoveSide::left, key.shiftPressed);
    case ARROW_RIGHT:
        return client.sendMsg(msg::Type::moveHorizontal, version, std::string{""}, msg::MoveSide::right, key.shiftPressed);
    case ARROW_UP:
        return client.sendMsg(msg::Type::moveVertical, version, std::string{""}, msg::MoveSide::up, terminal.getDocBufferWidth(), key.shiftPressed);
    case ARROW_DOWN:
        return client.sendMsg(msg::Type::moveVertical, version, std::string{""}, msg::MoveSide::down, terminal.getDocBufferWidth(), key.shiftPressed);
    case CTRL_A:
        return client.sendMsg(msg::Type::selectAll, version, std::string{""});
    case CTRL_C:
        return terminal.setClipboardData(doc.getSelectedText());
    case CTRL_V:
        return client.sendMsg(msg::Type::write, version, std::string{""}, terminal.getClipboardData());
    case CTRL_X:
        terminal.setClipboardData(doc.getSelectedText());
        return client.sendMsg(msg::Type::erase, version, std::string{""}, static_cast<unsigned int>(1));
    case CTRL_Z:
        if (key.shiftPressed) {
            return client.sendMsg(msg::Type::redo, version, std::string{""});
        }
        return client.sendMsg(msg::Type::undo, version, std::string{""});
    }
    return false;
}
