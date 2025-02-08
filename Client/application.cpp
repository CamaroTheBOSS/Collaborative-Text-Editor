#include <chrono>
#include <thread>

#include "window_text_editor.h"
#include "window_search.h"
#include "application.h"
#include "logging.h"

using namespace client;
constexpr msg::OneByteInt version = 1;

Application::Application() :
    client(),
    terminal(),
    windows(),
    repo() {
    COORD consoleSize = terminal.getScreenSize();
    ScrollableScreenBufferBuilder builder;
    builder.showLineNumbers()
        .setRelativeLeft(0.1)
        .setRelativeTop(0.1)
        .setRelativeRight(0.9)
        .setRelativeBot(0.9)
        .setConsoleSize(Pos<int>{consoleSize.X, consoleSize.Y})
        .showLeftFramePattern("|");
    windows.emplace_back(std::make_unique<TextEditorWindow>(builder));
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

void Application::changeFocusUp() {
    double curr = windows[focus]->getBuffer().getCenter().Y;
    for (int i = 0; i < windows.size(); i++) {
        double other = windows[i]->getBuffer().getCenter().Y;
        if (other <= curr) {
            curr = other;
            focus = i;
        }
    }
}

void Application::changeFocusDown() {
    double curr = windows[focus]->getBuffer().getCenter().Y;
    for (int i = 0; i < windows.size(); i++) {
        double other = windows[i]->getBuffer().getCenter().Y;
        if (other > curr) {
            curr = other;
            focus = i;
        }
    }
}

ActionDone Application::processChar(const KeyPack& key) {
    switch (key.keyCode) {
    case CTRL_V:
        return windows[focus]->processChar(client, key, terminal.getClipboardData());
    case CTRL_C:
        terminal.setClipboardData(windows[focus]->getDoc().getSelectedText());
        return ActionDone::done;
    case CTRL_X:
        terminal.setClipboardData(windows[focus]->getDoc().getSelectedText());
        return windows[focus]->processChar(client, key);
    }
    auto actionDone = windows[focus]->processChar(client, key);
    if (actionDone == ActionDone::up) {
        changeFocusUp();
    }
    else if (actionDone == ActionDone::down) {
        changeFocusDown();
    }
    return actionDone;
}

bool Application::checkBufferWasResized() {
    bool screenResized = terminal.resizeScreenBufferIfNeeded();
    if (screenResized) {
        COORD newConsoleSize = terminal.getScreenSize();
        windows[0]->getBuffer().setNewConsoleSize({ newConsoleSize.X, newConsoleSize.Y });
    }
    return screenResized;
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
    needRender += checkBufferWasResized();
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