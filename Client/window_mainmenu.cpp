#include "window_mainmenu.h"

#define CREATE 0
#define LOAD 1
#define HELP 2
#define QUIT 3

MainMenuWindow::MainMenuWindow() :
	BaseWindow() {
    doc.write(0, "Create Document\nLoad Document\nQuit");
    numOptions = doc.get().size();
    selectOption();
}

MainMenuWindow::MainMenuWindow(const ScrollableScreenBufferBuilder& ssbBuilder) :
	BaseWindow(ssbBuilder) {
    doc.write(0, "Create Document\nLoad Document\nHelp Control\nQuit");
    numOptions = doc.get().size();
    selectOption();
}

ActionDone MainMenuWindow::processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) {
    switch (key.keyCode) {
    case ENTER:
        return executeOption();
    case ARROW_UP:
        return goUp();
    case ARROW_DOWN:
        return goDown();
    }
    return ActionDone::undone;
}

ActionDone MainMenuWindow::goUp() {
    selectedOption--;
    if (selectedOption < 0) {
        selectedOption = numOptions - 1;
    }
    selectOption();
    return ActionDone::render;
}

ActionDone MainMenuWindow::goDown() {
    selectedOption++;
    if (selectedOption >= numOptions) {
        selectedOption = 0;
    }
    selectOption();
    return ActionDone::render;
}

void MainMenuWindow::selectOption() {
    doc.setCursorPos(0, COORD{ 0, (SHORT)selectedOption });
    doc.setCursorAnchor(0, COORD{ (SHORT)doc.get()[selectedOption].size(), (SHORT)selectedOption });
}

ActionDone MainMenuWindow::executeOption() {
    switch (selectedOption) {
    case CREATE:
        return ActionDone::createdoc;
    case LOAD:
        return ActionDone::loaddoc;
    case HELP:
        return ActionDone::help;
    case QUIT:
        exit(0);
    }
    return ActionDone::undone;
}
