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
    windows(),
    repo() {
    COORD consoleSize = terminal.getScreenSize();
    ScrollableScreenBufferBuilder builder;
    builder.showLineNumbers()
        .setTitle("Document")
        .setScrollHisteresis(2)
        .setRelativeLeft(0.1)
        .setRelativeTop(0.1)
        .setRelativeRight(0.9)
        .setRelativeBot(0.9)
        .setConsoleSize(Pos<int>{consoleSize.X, consoleSize.Y})
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    windows.emplace_back(std::make_unique<TextEditorWindow>(builder));
    windows[0]->activate();
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
    int newFocus = focus;
    float curr = windows[focus]->getBuffer().getCenter().Y;
    float diff = INT_MAX;
    for (int i = 0; i < windows.size(); i++) {
        float other = windows[i]->getBuffer().getCenter().Y;
        float newDiff = curr - other;
        if (newDiff > 0 && newDiff < diff) {
            diff = newDiff;
            newFocus = i;
        }
    }
    setFocus(newFocus);
}

void Application::changeFocusDown() {
    int newFocus = focus;
    float curr = windows[focus]->getBuffer().getCenter().Y;
    float diff = INT_MAX;
    for (int i = 0; i < windows.size(); i++) {
        float other = windows[i]->getBuffer().getCenter().Y;
        float newDiff = other - curr;
        if (newDiff > 0 && newDiff < diff) {
            diff = newDiff;
            newFocus = i;
        }
    }
    setFocus(newFocus);
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
    case CTRL_F:
    case F3:
        showSearchWindow();
        return ActionDone::render;
    case CTRL_R:
        if (key.shiftPressed) {
            showReplaceWindow();
        }
        return ActionDone::render;
    case ESC:
        destroyLastWindow();
        return ActionDone::render;
    }
    auto actionDone = windows[focus]->processChar(client, key);
    switch (actionDone) {
    case ActionDone::up:
        changeFocusUp();
        return ActionDone::render;
    case ActionDone::down:
        changeFocusDown();
        return ActionDone::render;
    }
}

bool Application::checkBufferWasResized() {
    bool screenResized = terminal.resizeScreenBufferIfNeeded();
    if (screenResized) {
        COORD newConsoleSize = terminal.getScreenSize();
        for (auto& window : windows) {
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
    terminal.render(windows);
}

void Application::setFocus(const int windowIdx) {
    if (windowIdx >= windows.size()) {
        return;
    }
    if (focus < windows.size()) {
        windows[focus]->deactivate();
    }
    focus = windowIdx;
    windows[focus]->activate();
}

WindowsIt Application::showSearchWindow() {
    if (windowsRegistry.find(SearchWindow::className) != windowsRegistry.cend()) {
        return findWindow(SearchWindow::className);
    }
    COORD consoleSize = terminal.getScreenSize();
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle(SearchWindow::className)
        .setRelativeLeft(0.7)
        .setRelativeTop(0.1)
        .setRelativeRight(0.9)
        .setRelativeBot(0.12)
        .setConsoleSize(Pos<int>{consoleSize.X, consoleSize.Y})
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    auto window = std::make_unique<SearchWindow>(builder, &windows[0]->getDoc());
    windowsRegistry[window->name()] = true;
    windows.emplace_back(std::move(window));
    setFocus(windows.size() - 1);
    return windows.cend() - 1;

}

WindowsIt Application::showReplaceWindow() {
    if (windowsRegistry.find(ReplaceWindow::className) != windowsRegistry.cend()) {
        return findWindow(ReplaceWindow::className);
    }
    auto& searchBuffer = showSearchWindow()->get()->getBuffer();
    COORD consoleSize = terminal.getScreenSize();
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle(ReplaceWindow::className)
        .setAbsoluteLeft(searchBuffer.getLeft())
        .setAbsoluteTop(searchBuffer.getBottom() + 2)
        .setAbsoluteRight(searchBuffer.getRight())
        .setAbsoluteBot(2 * searchBuffer.getBottom() - searchBuffer.getTop() + 2)
        .setConsoleSize(Pos<int>{consoleSize.X, consoleSize.Y})
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    auto window = std::make_unique<ReplaceWindow>(builder, &windows[0]->getDoc());
    windowsRegistry[window->name()] = true;
    windows.emplace_back(std::move(window));
    setFocus(windows.size() - 1);
    return windows.cend() - 1;
}

WindowsIt Application::findWindow(const std::string& name) const {
    for (auto it = windows.cbegin(); it != windows.cend(); it++) {
        if (it->get()->name() == name) {
            return it;
        }
    }
    windows.cend();
}

void Application::destroyLastWindow() {
    if (windows.size() <= 1) {
        return;
    }
    auto& last = windows.back();
    bool isActive = last->isActive();
    windowsRegistry.erase(last->name());
    windows.erase(windows.cend() - 1);
    if (isActive) {
        setFocus(windows.size() - 1);
    }
}