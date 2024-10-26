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

int Controller::readChar() const {
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
        return client.sendMsg(msg::Type::erase, version, std::string{""}, static_cast<unsigned int>(1));
    case ARROW_LEFT:
        return client.sendMsg(msg::Type::moveHorizontal, version, std::string{""}, msg::MoveSide::left);
    case ARROW_RIGHT:
        return client.sendMsg(msg::Type::moveHorizontal, version, std::string{""}, msg::MoveSide::right);
    case ARROW_UP:
        return client.sendMsg(msg::Type::moveVertical, version, std::string{""}, msg::MoveSide::up, terminal.getWidth());
    case ARROW_DOWN:
        return client.sendMsg(msg::Type::moveVertical, version, std::string{""}, msg::MoveSide::down, terminal.getWidth());
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