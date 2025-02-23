#include "window_helpers.h"
#include "window_text_editor.h"
#include "window_search.h"
#include "window_replace.h"
#include "window_createdoc.h"
#include "window_loaddoc.h"

ScrollableScreenBufferBuilder makeCreateDocWindowBuilder() {
    ScrollableScreenBufferBuilder builder;
    builder.setTitle("Enter name for document")
        .setScrollHisteresis(0)
        .setRelativeLeft(0.35)
        .setRelativeTop(0.44)
        .setRelativeRight(0.65)
        .setRelativeBot(0.56)
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}
ScrollableScreenBufferBuilder makeLoadDocWindowBuilder() {
    ScrollableScreenBufferBuilder builder;
    builder.setTitle("Enter access code")
        .setScrollHisteresis(0)
        .setRelativeLeft(0.35)
        .setRelativeTop(0.44)
        .setRelativeRight(0.65)
        .setRelativeBot(0.56)
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}
ScrollableScreenBufferBuilder makeSearchWindowBuilder() {
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle(SearchWindow::className)
        .setRelativeLeft(0.7)
        .setRelativeTop(0.1)
        .setRelativeRight(0.9)
        .setRelativeBot(0.12)
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}
ScrollableScreenBufferBuilder makeReplaceWindowBuilder() {
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle(ReplaceWindow::className)
        .setRelativeLeft(0.7)
        .setRelativeTop(0.2)
        .setRelativeRight(0.9)
        .setRelativeBot(0.22)
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}
ScrollableScreenBufferBuilder makeTextEditorWindowBuilder() {
    ScrollableScreenBufferBuilder builder;
    builder.showLineNumbers()
        .setTitle("Document")
        .setScrollHisteresis(2)
        .setRelativeLeft(0.1)
        .setRelativeTop(0.1)
        .setRelativeRight(0.9)
        .setRelativeBot(0.9)
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}
ScrollableScreenBufferBuilder makeInfoWindowBuilder(const std::string& title) {
    ScrollableScreenBufferBuilder builder;
    builder.setTitle(title)
        .setScrollHisteresis(0)
        .setRelativeLeft(0.30)
        .setRelativeTop(0.35)
        .setRelativeRight(0.7)
        .setRelativeBot(0.65)
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}

ScrollableScreenBufferBuilder makeMenuWindowBuilder(const std::string& title) {
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle(title)
        .setRelativeLeft(0.35)
        .setRelativeTop(0.4)
        .setRelativeRight(0.65)
        .setRelativeBot(0.6)
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}

std::vector<Option> makeMainMenuOptions() {
    return std::vector<Option>{
        Option{ "Create document", [](MenuWindow& obj) { return Event{ windows::app::events::createDocWindow, obj.name(), windows::app::name, {} }; } },
        Option{ "Load document", [](MenuWindow& obj) { return Event{ windows::app::events::loadDocWindow, obj.name(), windows::app::name, {} }; } },
        Option{ "Help", [](MenuWindow& obj) { return Event{ windows::app::events::help, obj.name(), windows::app::name, {} }; } },
        Option{ "Quit", [](MenuWindow& obj) { return Event{ windows::app::events::exit, obj.name(), windows::app::name, {} }; } }
    };
}
std::vector<Option> makeLoggedMainMenuOptions() {
    return std::vector<Option>{
        Option{ "Disconnect", [](MenuWindow& obj) { return Event{ windows::app::events::disconnect, obj.name(), windows::app::name, {} }; } },
        Option{ "Show access code", [](MenuWindow& obj) { return Event{ windows::app::events::showAcCode, obj.name(), windows::app::name, {} }; } },
        Option{ "Create document", [](MenuWindow& obj) { return Event{ windows::app::events::createDocWindow, obj.name(), windows::app::name, {} }; } },
        Option{ "Load document", [](MenuWindow& obj) { return Event{ windows::app::events::loadDocWindow, obj.name(), windows::app::name, {} }; } },
        Option{ "Help", [](MenuWindow& obj) { return Event{ windows::app::events::help, obj.name(), windows::app::name, {} }; } },
        Option{ "Quit", [](MenuWindow& obj) { return Event{ windows::app::events::exit, obj.name(), windows::app::name, {} }; } }
    };
}

std::string getHelpWindowText() {
    return
        "Collaborative Text Editor!\n"
        "Choose 'Create Document' to create document on the server\n"
        "Choose 'Load Document' to connect to existing document\n"
        "F3/CTRL+F - find\n"
        "CTRL+SHIFT+F - find and replace\n"
        "CTRL+Q - open main menu\n"
        "ESC - close last window\n"
        "CTRL+Arrow(up/down/left/right) - change active window\n\n"
        "Credits\n"
        "Main programmer: Kacper Plesiak\n"
        "Main designer: Kacper Plesiak\n"
        "LICENSE: MIT\n"
        "C++ is cool\n";
}