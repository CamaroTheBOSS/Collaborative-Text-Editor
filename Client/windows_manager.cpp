#include "windows_manager.h"


void WindowsManager::changeFocusUp() {
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

void WindowsManager::changeFocusDown() {
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

void WindowsManager::changeFocusLeft() {
    int newFocus = focus;
    float curr = windows[focus]->getBuffer().getCenter().X;
    float diff = INT_MAX;
    for (int i = 0; i < windows.size(); i++) {
        float other = windows[i]->getBuffer().getCenter().X;
        float newDiff = curr - other;
        if (newDiff > 0 && newDiff < diff) {
            diff = newDiff;
            newFocus = i;
        }
    }
    setFocus(newFocus);
}

void WindowsManager::changeFocusRight() {
    int newFocus = focus;
    float curr = windows[focus]->getBuffer().getCenter().X;
    float diff = INT_MAX;
    for (int i = 0; i < windows.size(); i++) {
        float other = windows[i]->getBuffer().getCenter().X;
        float newDiff = other - curr;
        if (newDiff > 0 && newDiff < diff) {
            diff = newDiff;
            newFocus = i;
        }
    }
    setFocus(newFocus);
}

void WindowsManager::setFocus(const int newFocus) {
    if (newFocus >= windows.size()) {
        return;
    }
    if (focus < windows.size()) {
        windows[focus]->deactivate();
    }
    focus = newFocus;
    windows[focus]->activate();
}

WindowsIt WindowsManager::showMainMenuWindow(const COORD& consoleSize) {
    if (windowsRegistry.find(MainMenuWindow::className) != windowsRegistry.cend()) {
        return findWindow(MainMenuWindow::className);
    }
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle(MainMenuWindow::className)
        .setRelativeLeft(0.35)
        .setRelativeTop(0.4)
        .setRelativeRight(0.65)
        .setRelativeBot(0.6)
        .setConsoleSize(Pos<int>{consoleSize.X, consoleSize.Y})
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    auto window = std::make_unique<MainMenuWindow>(builder);
    windowsRegistry[window->name()] = true;
    windows.emplace_back(std::move(window));
    setFocus(windows.size() - 1);
    return windows.cend() - 1;
}

WindowsIt WindowsManager::showSearchWindow(const COORD& consoleSize) {
    if (windowsRegistry.find(SearchWindow::className) != windowsRegistry.cend()) {
        return findWindow(SearchWindow::className);
    }
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
    auto window = std::make_unique<SearchWindow>(builder);
    windowsRegistry[window->name()] = true;
    windows.emplace_back(std::move(window));
    setFocus(windows.size() - 1);
    return windows.cend() - 1;
}

WindowsIt WindowsManager::showReplaceWindow(const COORD& consoleSize) {
    if (windowsRegistry.find(ReplaceWindow::className) != windowsRegistry.cend()) {
        return findWindow(ReplaceWindow::className);
    }
    auto& searchBuffer = showSearchWindow(consoleSize)->get()->getBuffer();
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
    auto window = std::make_unique<ReplaceWindow>(builder);
    windowsRegistry[window->name()] = true;
    windows.emplace_back(std::move(window));
    setFocus(windows.size() - 1);
    return windows.cend() - 1;
}

WindowsIt WindowsManager::showTextEditorWindow(const COORD& consoleSize) {
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
    return windows.cbegin();
}

void WindowsManager::destroyLastWindow(const TCPClient& client) {
    if (windows.size() <= 1) {
        return;
    }
    auto& last = windows.back();
    auto pEvent = last->onDelete();
    processEvent(client, pEvent);
    bool isActive = last->isActive();
    windowsRegistry.erase(last->name());
    windows.erase(windows.cend() - 1);
    if (isActive) {
        setFocus(windows.size() - 1);
    }
}

const Window& WindowsManager::getFocusedWindow() const {
    return windows[focus];
}

const Window& WindowsManager::getTextEditor() const {
    return windows[0];
}

const Windows& WindowsManager::getWindows() const {
    return windows;
}

bool WindowsManager::processEvent(const TCPClient& client, const Event& pEvent) {
    auto it = findWindow(pEvent.target);
    if (it == windows.cend()) {
        return false;
    }
    it->get()->processEvent(client, pEvent);
    return true;
}

WindowsIt WindowsManager::findWindow(const std::string& name) const {
    for (auto it = windows.cbegin(); it != windows.cend(); it++) {
        if (it->get()->name() == name) {
            return it;
        }
    }
    windows.cend();
}