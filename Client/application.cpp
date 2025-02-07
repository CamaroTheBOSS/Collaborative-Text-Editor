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
    windows(),
    repo() {
    windows.emplace_back(std::make_unique<TextEditorWindow>());
    terminal.resizeScreenBufferIfNeeded(windows[0]);
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
        return windows[focus]->processChar(client, key, terminal.getClipboardData());
    case CTRL_C:
        return terminal.setClipboardData(windows[focus]->getDoc().getSelectedText());
    case CTRL_X:
        terminal.setClipboardData(windows[focus]->getDoc().getSelectedText());
        return windows[focus]->processChar(client, key);
    }
    return windows[focus]->processChar(client, key);
}

bool Application::checkIncomingMessages() {
    int needRender = 0;
    while (true) {
        msg::Buffer msgBuffer = client.getNextMsg();
        if (msgBuffer.empty()) {
            break;
        }
        needRender += repo.processMsg(windows[0]->getDoc(), msgBuffer);
    }
    needRender += terminal.resizeScreenBufferIfNeeded(windows[0]);
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
        repo.processMsg(windows[0]->getDoc(), msgBuffer);
        if (msgType == msg::Type::sync) {
            return true;
        }
    }
    return false;
}

void Application::render() {
    for (auto& window : windows) {
        terminal.render(window);
    }
}