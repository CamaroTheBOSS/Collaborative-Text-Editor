#include "windows_manager.h"
#include "pos_helpers.h"


WindowsManager::WindowsManager(const COORD& consoleSize) {
    builderMap[TextEditorWindow::className] = makeTextEditorWindowBuilder(consoleSize);
}

void WindowsManager::changeFocusUp() {
    return changeFocus(ChangeFocusDirection::up);
}

void WindowsManager::changeFocusDown() {
    return changeFocus(ChangeFocusDirection::down);
}

void WindowsManager::changeFocusLeft() {
    return changeFocus(ChangeFocusDirection::left);
}

void WindowsManager::changeFocusRight() {
    return changeFocus(ChangeFocusDirection::right);
}

void WindowsManager::setFocus(const std::string& winName) {
    auto window = findWindow(winName);
    if (window == windows.cend()) {
        return;
    }
    const int newFocus = std::distance(windows.cbegin(), window);
    return setFocus(newFocus);
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

void WindowsManager::destroyWindow(const std::string& name, const TCPClient& client) {
    if (windows.size() <= 1) {
        return;
    }
    auto window = findWindow(name);
    if (window == windows.cend()) {
        return;
    }
    auto pEvent = window->get()->onDelete();
    processEvent(client, pEvent);
    std::size_t index = std::distance(windows.cbegin(), window);
    bool needToChangeFocus = focus >= index;
    windowsRegistry.erase(window->get()->name());
    windows.erase(window);
    if (needToChangeFocus) {
        setFocus(focus - 1);
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

void WindowsManager::changeFocus(const ChangeFocusDirection direction) {
    int diffMultiplier = 2 * (direction == ChangeFocusDirection::left || direction == ChangeFocusDirection::up) - 1;
    bool isVertical = (direction == ChangeFocusDirection::down || direction == ChangeFocusDirection::up);

    int newFocus = focus;
    auto currentPos = windows[focus]->getFocusAnchor();
    auto diffPosRelevant = SHRT_MAX;
    int distanceBetween = SHRT_MAX;
    for (int i = 0; i < windows.size(); i++) {
        auto otherPos = windows[i]->getFocusAnchor();
        auto newDiffPos = (currentPos - otherPos) * diffMultiplier;
        auto newDiffPosRelevant = isVertical ? newDiffPos.Y : newDiffPos.X;
        if (newDiffPosRelevant <= 0) {
            continue;
        }
        if (newDiffPosRelevant < diffPosRelevant) {
            newFocus = i;
            diffPosRelevant = newDiffPosRelevant;
            distanceBetween = abs(newDiffPos.X) + abs(newDiffPos.Y);
        }
        else if (newDiffPosRelevant == diffPosRelevant) {
            int distanceBetween2 = abs(newDiffPos.X) + abs(newDiffPos.Y);
            if (distanceBetween2 < distanceBetween) {
                newFocus = i;
                diffPosRelevant = newDiffPosRelevant;
                distanceBetween = distanceBetween2;
            }
        }
    }
    setFocus(newFocus);
}
