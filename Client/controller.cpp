#include "controller.h"

constexpr msg::OneByteInt version = 1;

bool Controller::connect(const std::string& ip, const int port) {
    return client.connectServer(ip, port);
}

bool Controller::disconnect() {
	return client.disconnect();
}

int Controller::readChar() {
    return terminal.readChar();
}

bool Controller::processChar(const int key) {
    
    if (key >= 32 && key <= 127) {
        return client.sendMsg(msg::Type::write, version, std::string{""}, static_cast<msg::OneByteInt>(key));
    }
    switch (key) {
    case ENTER:
        return client.sendMsg(msg::Type::write, version, std::string{""}, std::string{'\n'});
    case TABULAR:
        return client.sendMsg(msg::Type::write, version, std::string{""}, std::string{"    "});
    case BACKSPACE:
        return client.sendMsg(msg::Type::erase, version, std::string{""}, 1);
    case ARROW_LEFT:
        return client.sendMsg(msg::Type::moveHorizontal, version, std::string{""}, msg::MoveSide::left);
    case ARROW_RIGHT:
        return client.sendMsg(msg::Type::moveHorizontal, version, std::string{""}, msg::MoveSide::right);
    case ARROW_UP:
        return client.sendMsg(msg::Type::moveVertical, version, std::string{""}, msg::MoveSide::up, terminal.getWidth());
    case ARROW_DOWN:
        return client.sendMsg(msg::Type::moveVertical, version, std::string{""}, msg::MoveSide::down, terminal.getWidth());
    }
    return false;
}

void Controller::checkIncomingMessages() {
    while (true) {
        msg::Buffer msgBuffer = client.getNextMsg();
        if (msgBuffer.empty()) {
            break;
        }
        repo.processMsg(msgBuffer);
    }
}

void Controller::render() {
    terminal.render(repo.getDoc());
}