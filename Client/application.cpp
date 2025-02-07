#include <chrono>
#include <thread>

#include "window_text_editor.h"
#include "application.h"
#include "logging.h"

using namespace client;
constexpr msg::OneByteInt version = 1;

Application::Application() :
    client(),
    terminal(),
    window(std::make_unique<TextEditorWindow>(client)),
    repo(window->getDoc()) {
    terminal.resizeScreenBufferIfNeeded(window);
}

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
    switch (key.keyCode) {
    case CTRL_V:
        return window->processChar(key, terminal.getClipboardData());
    case CTRL_C:
        return terminal.setClipboardData(window->getDoc().getSelectedText());
    case CTRL_X:
        terminal.setClipboardData(window->getDoc().getSelectedText());
        return window->processChar(key);
    }
    return window->processChar(key);
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
    needRender += terminal.resizeScreenBufferIfNeeded(window);
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
    terminal.render(window);
}