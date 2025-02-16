#include <chrono>
#include <thread>

#include "window_text_editor.h"
#include "window_search.h"
#include "window_replace.h"
#include "application.h"
#include "logging.h"

using namespace client;
constexpr msg::OneByteInt version = 1;

Application::Application() :
    client(),
    terminal(),
    windowsManager(),
    repo() {
    windowsManager.showTextEditorWindow(terminal.getScreenSize());
    windowsManager.showMainMenuWindow(terminal.getScreenSize());
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

ActionDone Application::processChar(const KeyPack& key) {
    auto& window = windowsManager.getFocusedWindow();
    switch (key.keyCode) {
    case CTRL_ARROW_DOWN:
        windowsManager.changeFocusDown();
        return ActionDone::render;
    case CTRL_ARROW_UP:
        windowsManager.changeFocusUp();
        return ActionDone::render;
    case CTRL_ARROW_LEFT:
        windowsManager.changeFocusLeft();
        return ActionDone::render;
    case CTRL_ARROW_RIGHT:
        windowsManager.changeFocusRight();
        return ActionDone::render;
    case CTRL_V:
        return window->processChar(client, key, terminal.getClipboardData());
    case CTRL_C:
        terminal.setClipboardData(window->getDoc().getSelectedText());
        return ActionDone::done;
    case CTRL_X:
        terminal.setClipboardData(window->getDoc().getSelectedText());
        return window->processChar(client, key);
    case CTRL_F:
    case F3:
        windowsManager.showSearchWindow(terminal.getScreenSize());
        return ActionDone::render;
    case CTRL_R:
        if (key.shiftPressed) {
            windowsManager.showReplaceWindow(terminal.getScreenSize());
        }
        return ActionDone::render;
    case CTRL_Q:
        windowsManager.showMainMenuWindow(terminal.getScreenSize());
        return ActionDone::render;
    case ESC:
        windowsManager.destroyLastWindow();
        return ActionDone::render;
    }
    ActionDone action = window->processChar(client, key);
    if (action == ActionDone::createdoc || action == ActionDone::loaddoc) {
        requestDocument(std::chrono::milliseconds(500), 4000);
        windowsManager.destroyLastWindow();
        return ActionDone::render;
    }
    return action;
}

bool Application::checkBufferWasResized() {
    bool screenResized = terminal.resizeScreenBufferIfNeeded();
    if (screenResized) {
        COORD newConsoleSize = terminal.getScreenSize();
        for (const auto& window : windowsManager.getWindows()) {
            window->getBuffer().setNewConsoleSize({ newConsoleSize.X, newConsoleSize.Y });
        }
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
        needRender += repo.processMsg(windowsManager.getTextEditor()->getDoc(), msgBuffer);
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
        repo.processMsg(windowsManager.getTextEditor()->getDoc(), msgBuffer);
        if (msgType == msg::Type::sync) {
            return true;
        }
    }
    logger.logDebug(currTry, "Requesting document from the server failed!\n");
    return false;
}

void Application::render() {
    terminal.render(windowsManager.getWindows());
}
