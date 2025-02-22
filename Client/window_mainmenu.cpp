#include "window_mainmenu.h"

#define CREATE 0
#define LOAD 1
#define HELP 2
#define QUIT 3

MainMenuWindow::MainMenuWindow(const ScrollableScreenBufferBuilder& ssbBuilder) :
	BaseWindow(ssbBuilder) {
    doc.write(0, "Create Document\nLoad Document\nHelp Control\nQuit");
    numOptions = doc.get().size();
    selectOption();
}

Event MainMenuWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
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

Event MainMenuWindow::goUp() {
    selectedOption--;
    if (selectedOption < 0) {
        selectedOption = numOptions - 1;
    }
    selectOption();
    return Event{};
}

Event MainMenuWindow::goDown() {
    selectedOption++;
    if (selectedOption >= numOptions) {
        selectedOption = 0;
    }
    selectOption();
    return Event{};
}

void MainMenuWindow::selectOption() {
    doc.setCursorPos(0, COORD{ 0, (SHORT)selectedOption });
    doc.setCursorAnchor(0, COORD{ (SHORT)doc.get()[selectedOption].size(), (SHORT)selectedOption });
}

Event MainMenuWindow::executeOption() {
    switch (selectedOption) {
    case CREATE:
        return Event{ windows::app::events::createDocWindow, className, windows::app::name, {}};
    case LOAD:
        return Event{ windows::app::events::loadDocWindow, className, windows::app::name, {} };
    case HELP:
        return Event{ windows::app::events::help, className, windows::app::name, {} };
    case QUIT:
        return Event{ windows::app::events::exit, className, windows::app::name, {} };
    }
    return Event{};
}
