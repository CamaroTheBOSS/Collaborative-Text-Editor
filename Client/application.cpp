#include <chrono>
#include <thread>

#include "controller_doc.h"
#include "application.h"
#include "logging.h"

using namespace client;
constexpr msg::OneByteInt version = 1;

Application::Application() :
    client(),
    terminal(),
    controller(std::make_unique<DocController>(client, terminal)),
    repo(controller->getDoc()) {}

bool Application::connect(const std::string& ip, const int port) {
    return client.connectServer(ip, port);
}

bool Application::disconnect() {
    return client.disconnect();
}

bool Application::isConnected() const {
    return client.isConnected();
}

KeyPack Application::readChar() const {
    return terminal.readChar();
}

bool Application::processChar(const KeyPack& key) {
    return controller->processChar(key);
}

bool Application::checkIncomingMessages() {
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

bool Application::requestDocument(const std::chrono::milliseconds& timeout, const int tries) {
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

void Application::render() {
    /*for (auto& window : windows) {
        terminal.render(window.getDoc());
    }*/
    terminal.render(controller->getDoc());
}