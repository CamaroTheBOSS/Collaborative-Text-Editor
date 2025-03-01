#include "window_text_editor.h"

constexpr msg::OneByteInt version = 1;

TextEditorWindow::TextEditorWindow(const ScrollableScreenBufferBuilder& ssbBuilder) :
    BaseWindow(ssbBuilder) {
    eventHandlers.try_emplace(windows::text_editor::events::find, &TextEditorWindow::find);
    eventHandlers.try_emplace(windows::text_editor::events::findNext, &TextEditorWindow::findNext);
    eventHandlers.try_emplace(windows::text_editor::events::findReset, &TextEditorWindow::findReset);
    eventHandlers.try_emplace(windows::text_editor::events::replace, &TextEditorWindow::replace);
}

Event TextEditorWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        client.sendMsg(msg::Type::write, version, authToken, std::string(1, key.keyCode));
        return Event{};
    }
    bool actionDone = false;
    switch (key.keyCode) {
    case ENTER:
        actionDone = client.sendMsg(msg::Type::write, version, authToken, std::string{'\n'});
        break;
    case TABULAR:
        actionDone = client.sendMsg(msg::Type::write, version, authToken, std::string{"    "});
        break;
    case BACKSPACE:
        actionDone = client.sendMsg(msg::Type::erase, version, authToken, static_cast<unsigned int>(1));
        break;
    case ARROW_LEFT:
        actionDone = client.sendMsg(msg::Type::moveHorizontal, version, authToken, msg::MoveSide::left, key.shiftPressed);
        break;
    case ARROW_RIGHT:
        actionDone = client.sendMsg(msg::Type::moveHorizontal, version, authToken, msg::MoveSide::right, key.shiftPressed);
        break;
    case ARROW_UP:
        actionDone = client.sendMsg(msg::Type::moveVertical, version, authToken, msg::MoveSide::up, getDocBufferWidth(), key.shiftPressed);
        break;
    case ARROW_DOWN:
        actionDone = client.sendMsg(msg::Type::moveVertical, version, authToken, msg::MoveSide::down, getDocBufferWidth(), key.shiftPressed);
        break;
    case CTRL_A:
        actionDone = client.sendMsg(msg::Type::selectAll, version, authToken);
        break;
    case CTRL_V:
        actionDone = client.sendMsg(msg::Type::write, version, authToken, clipboardData);
        break;
    case CTRL_X:
        actionDone = client.sendMsg(msg::Type::erase, version, authToken, static_cast<unsigned int>(1));
        break;
    case CTRL_Z:
        if (key.shiftPressed) {
            actionDone = client.sendMsg(msg::Type::redo, version, authToken);
        }
        else {
            actionDone = client.sendMsg(msg::Type::undo, version, authToken);
        }
        break;
    }
    return Event{};
}

void TextEditorWindow::processEvent(const TCPClient& client, const Event& pEvent) {
    auto it = eventHandlers.find(pEvent.name);
    if (it == eventHandlers.cend()) {
        return;
    }
    return (this->*it->second)(client, pEvent.params);
}

void TextEditorWindow::setAuthToken(const std::string& newAuthToken) {
    authToken = newAuthToken;
}

void TextEditorWindow::find(const TCPClient& client, const std::vector<std::string>& args) {
    if (args.empty()) {
        return;
    }
    doc.findSegments(args[0]);
}

void TextEditorWindow::findReset(const TCPClient& client, const std::vector<std::string>& args) {
    doc.resetSegments();
}

void TextEditorWindow::findNext(const TCPClient& client, const std::vector<std::string>& args) {
    COORD pos = doc.getNextSegmentPos();
    if (pos == COORD{-1, -1}) {
        return;
    }
    unsigned int X = pos.X;
    unsigned int Y = pos.Y;
    client.sendMsg(msg::Type::moveTo, version, std::string{""}, X, Y);
}

void TextEditorWindow::replace(const TCPClient& client, const std::vector<std::string>& args) {
    if (doc.getSegments().empty() || args.empty()) {
        return;
    }
    client.sendMsg(msg::Type::replace, version, std::string{""}, args[0], doc.getSegments());
}


