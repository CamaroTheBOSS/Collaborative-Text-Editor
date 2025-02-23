#include "window_menu.h"

MenuWindow::MenuWindow(const ScrollableScreenBufferBuilder& ssbBuilder, const std::string& title, std::vector<Option>&& menuOptions) :
    BaseWindow(ssbBuilder),
    className(title),
    options(std::move(menuOptions)) {
    for (const auto& option : options) {
        doc.write(0, option.name + "\n");
    }
    selectOption();
}

Event MenuWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    switch (key.keyCode) {
    case ENTER:
        return executeOption();
    case ARROW_UP:
        return goUp();
    case ARROW_DOWN:
        return goDown();
    }
    return Event{};
}

Event MenuWindow::goUp() {
    selectedOption--;
    if (selectedOption < 0) {
        selectedOption = options.size() - 1;
    }
    selectOption();
    return Event{};
}

Event MenuWindow::goDown() {
    selectedOption++;
    if (selectedOption >= options.size()) {
        selectedOption = 0;
    }
    selectOption();
    return Event{};
}

void MenuWindow::selectOption() {
    doc.setCursorPos(0, COORD{ 0, (SHORT)selectedOption });
    doc.setCursorAnchor(0, COORD{ (SHORT)doc.get()[selectedOption].size(), (SHORT)selectedOption });
}

Event MenuWindow::executeOption() {
    if (selectedOption < 0 || selectedOption >= options.size()) {
        return Event{};
    }
    return options[selectedOption].handler(*this);
}
