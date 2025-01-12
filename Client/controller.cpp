#include <chrono>
#include <thread>

#include "controller.h"
#include "logging.h"

constexpr msg::OneByteInt version = 1;

bool Controller::connect(const std::string& ip, const int port) {
    return client.connectServer(ip, port);
}

bool Controller::disconnect() {
	return client.disconnect();
}

bool Controller::isConnected() const {
    return client.isConnected();
}

KeyPack Controller::readChar() const {
    return terminal.readChar();
}

bool Controller::processChar(const KeyPack& key) {
    
    if (key.keyCode >= 32 && key.keyCode <= 127) {
        return client.sendMsg(msg::Type::write, version, authToken, std::string(1, key.keyCode));
    }
    switch (key.keyCode) {
    case ENTER:
        return client.sendMsg(msg::Type::write, version, authToken, std::string{'\n'});
    case TABULAR:
        return client.sendMsg(msg::Type::write, version, authToken, std::string{"    "});
    case BACKSPACE:
        return client.sendMsg(msg::Type::erase, version, authToken, static_cast<unsigned int>(1));
    case ARROW_LEFT:
        return client.sendMsg(msg::Type::moveHorizontal, version, authToken, msg::MoveSide::left, key.shiftPressed);
    case ARROW_RIGHT:
        return client.sendMsg(msg::Type::moveHorizontal, version, authToken, msg::MoveSide::right, key.shiftPressed);
    case ARROW_UP:
        return client.sendMsg(msg::Type::moveVertical, version, authToken, msg::MoveSide::up, terminal.getDocBufferWidth(), key.shiftPressed);
    case ARROW_DOWN:
        return client.sendMsg(msg::Type::moveVertical, version, authToken, msg::MoveSide::down, terminal.getDocBufferWidth(), key.shiftPressed);
    case CTRL_A:
        return client.sendMsg(msg::Type::selectAll, version, authToken);
    case CTRL_C:
        return terminal.setClipboardData(repo.getDoc().getSelectedText());
    case CTRL_V:
        return client.sendMsg(msg::Type::write, version, authToken, terminal.getClipboardData());
    case CTRL_X:
        terminal.setClipboardData(repo.getDoc().getSelectedText());
        return client.sendMsg(msg::Type::erase, version, authToken, static_cast<unsigned int>(1));
    case CTRL_Z:
        if (key.shiftPressed) {
            return client.sendMsg(msg::Type::redo, version, authToken);
        }
        return client.sendMsg(msg::Type::undo, version, authToken);
    case ESC:
        return disconnect();
    }
    return false;
}

bool Controller::checkIncomingMessages() {
    int needRender = 0;
    while (true) {
        msg::Buffer msgBuffer = client.getNextMsg();
        if (msgBuffer.empty()) {
            break;
        }
        needRender += repo.processMsg(msgBuffer);
    }
    needRender += terminal.resizeScreenBufferIfNeeded();
    return needRender;
}

bool Controller::requestDocument(const std::chrono::milliseconds& timeout, const int tries) {
    int currTry = 0;
    client.sendMsg(msg::Type::sync, version);
    while (currTry < tries) {
        msg::Buffer msgBuffer = client.getNextMsg();
        if (msgBuffer.empty()) {
            currTry++;
            logger.logDebug(currTry, " try to request document failed");
            std::this_thread::sleep_for(timeout);
            continue;
        }
        msg::Type msgType;
        msg::parse(msgBuffer, 0, msgType);
        repo.processMsg(msgBuffer);     
        if (msgType == msg::Type::sync) {
            return true;
        }
    }
    return false;
}

void Controller::render() {
    terminal.render(repo.getDoc());
}

bool Controller::saveDoc() const {
    return repo.saveDoc();
}