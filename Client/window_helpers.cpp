#include "window_helpers.h"

ScrollableScreenBufferBuilder makeCreateDocWindowBuilder(const COORD& consoleSize) {
    ScrollableScreenBufferBuilder builder;
    builder.setTitle("Enter name for document")
        .setScrollHisteresis(0)
        .setRelativeLeft(0.35)
        .setRelativeTop(0.44)
        .setRelativeRight(0.65)
        .setRelativeBot(0.56)
        .setConsoleSize({consoleSize.X, consoleSize.Y})
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}
ScrollableScreenBufferBuilder makeLoadDocWindowBuilder(const COORD& consoleSize) {
    ScrollableScreenBufferBuilder builder;
    builder.setTitle("Enter access code")
        .setScrollHisteresis(0)
        .setRelativeLeft(0.35)
        .setRelativeTop(0.44)
        .setRelativeRight(0.65)
        .setRelativeBot(0.56)
        .setConsoleSize({ consoleSize.X, consoleSize.Y })
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}
ScrollableScreenBufferBuilder makeSearchWindowBuilder(const COORD& consoleSize) {
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle("Search Window")
        .setRelativeLeft(0.7)
        .setRelativeTop(0.1)
        .setRelativeRight(0.9)
        .setRelativeBot(0.12)
        .setConsoleSize({ consoleSize.X, consoleSize.Y })
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}
ScrollableScreenBufferBuilder makeReplaceWindowBuilder(const COORD& consoleSize) {
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle("Replace Window")
        .setRelativeLeft(0.7)
        .setRelativeTop(0.2)
        .setRelativeRight(0.9)
        .setRelativeBot(0.22)
        .setConsoleSize({ consoleSize.X, consoleSize.Y })
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}
ScrollableScreenBufferBuilder makeTextEditorWindowBuilder(const COORD& consoleSize) {
    ScrollableScreenBufferBuilder builder;
    builder.showLineNumbers()
        .setTitle("Document")
        .setScrollHisteresis(2)
        .setRelativeLeft(0.1)
        .setRelativeTop(0.1)
        .setRelativeRight(0.9)
        .setRelativeBot(0.9)
        .setConsoleSize({ consoleSize.X, consoleSize.Y })
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}
ScrollableScreenBufferBuilder makeInfoWindowBuilder(const COORD& consoleSize, const std::string& title) {
    ScrollableScreenBufferBuilder builder;
    builder.setTitle(title)
        .setScrollHisteresis(0)
        .setRelativeLeft(0.30)
        .setRelativeTop(0.35)
        .setRelativeRight(0.7)
        .setRelativeBot(0.65)
        .setConsoleSize({ consoleSize.X, consoleSize.Y })
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}

ScrollableScreenBufferBuilder makeMenuWindowBuilder(const COORD& consoleSize, const std::string& title) {
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle(title)
        .setRelativeLeft(0.35)
        .setRelativeTop(0.4)
        .setRelativeRight(0.65)
        .setRelativeBot(0.6)
        .setConsoleSize({ consoleSize.X, consoleSize.Y })
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}

ScrollableScreenBufferBuilder makeGenericBuilder(const COORD& consoleSize, const std::string& title, const double left, const double top, const int width, const int height) {
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle(title)
        .setRelativeLeft(left)
        .setRelativeTop(top)
        .setAbsoluteWidth(width)
        .setAbsoluteHeight(height)
        .setConsoleSize({ consoleSize.X, consoleSize.Y })
        .showLeftFramePattern("|")
        .showRightFramePattern("|")
        .showTopFramePattern("-")
        .showBottomFramePattern("-");
    return builder;
}

std::vector<Option> makeUnloggedMainMenuOptions() {
    return std::vector<Option>{
        Option{ "Login", [](MenuWindow& obj) { return Event{ windows::app::events::showLoginWindow, obj.name(), windows::app::name, { windows::login::name }}; } },
        Option{ "Register", [](MenuWindow& obj) { return Event{ windows::app::events::showLoginWindow, obj.name(), windows::app::name, { windows::registration::name }}; }},
        Option{ "Help", [](MenuWindow& obj) { return Event{ windows::app::events::help, obj.name(), windows::app::name, {} }; } },
        Option{ "Quit", [](MenuWindow& obj) { return Event{ windows::app::events::exit, obj.name(), windows::app::name, {} }; } }
    };
}
std::vector<Option> makeLoggedMainMenuOptions() {
    return std::vector<Option>{
        Option{ "Logout", [](MenuWindow& obj) { return Event{ windows::app::events::disconnect, obj.name(), windows::app::name, {} }; } },
        Option{ "Create document", [](MenuWindow& obj) { return Event{ windows::app::events::createDocWindow, obj.name(), windows::app::name, {} }; } },
        Option{ "Load document", [](MenuWindow& obj) { return Event{ windows::app::events::joinDocWindow, obj.name(), windows::app::name, {} }; } },
        Option{ "Help", [](MenuWindow& obj) { return Event{ windows::app::events::help, obj.name(), windows::app::name, {} }; } },
        Option{ "Quit", [](MenuWindow& obj) { return Event{ windows::app::events::exit, obj.name(), windows::app::name, {} }; } }
    };
}
std::vector<Option> makeConnectedToDocMainMenuOptions() {
    return std::vector<Option>{
        Option{ "Logout", [](MenuWindow& obj) { return Event{ windows::app::events::disconnect, obj.name(), windows::app::name, {} }; } },
        Option{ "Show access code", [](MenuWindow& obj) { return Event{ windows::app::events::showAcCode, obj.name(), windows::app::name, {} }; } },
        Option{ "Create document", [](MenuWindow& obj) { return Event{ windows::app::events::createDocWindow, obj.name(), windows::app::name, {} }; } },
        Option{ "Load document", [](MenuWindow& obj) { return Event{ windows::app::events::joinDocWindow, obj.name(), windows::app::name, {} }; } },
        Option{ "Help", [](MenuWindow& obj) { return Event{ windows::app::events::help, obj.name(), windows::app::name, {} }; } },
        Option{ "Quit", [](MenuWindow& obj) { return Event{ windows::app::events::exit, obj.name(), windows::app::name, {} }; } }
    };
}

TextInputWindow::TextInputHandler funcSearchModifyEvent() {
    return [](const TextInputWindow& win, const ClientSiteDocument& doc) {
        return Event{ windows::text_editor::events::find, win.name(), windows::text_editor::name, { doc.getText()} };
    };
}
TextInputWindow::TextInputHandler funcSearchSubmitEvent() {
    return [](const TextInputWindow& win, const ClientSiteDocument& doc) {
        return Event{ windows::text_editor::events::findNext, win.name(), windows::text_editor::name, {} };
    };
}
TextInputWindow::TextInputHandler funcSearchDeleteEvent() {
    return [](const TextInputWindow& win, const ClientSiteDocument& doc) {
        return Event{ windows::text_editor::events::findReset, win.name(), windows::text_editor::name, {} };
    };
}
TextInputWindow::TextInputHandler funcReplaceSubmitEvent() {
    return [](const TextInputWindow& win, const ClientSiteDocument& doc) {
        return Event{ windows::text_editor::events::replace, win.name(), windows::text_editor::name, { doc.getText() } };
    };
}
TextInputWindow::TextInputHandler funcLoadDocSubmitEvent() {
    return [](const TextInputWindow& win, const ClientSiteDocument& doc) {
        return Event{ windows::app::events::joinDoc, win.name(), windows::app::name, { doc.getText() } };
    };
}
TextInputWindow::TextInputHandler funcCreateDocSubmitEvent() {
    return [](const TextInputWindow& win, const ClientSiteDocument& doc) {
        return Event{ windows::app::events::createDoc, win.name(), windows::app::name, { doc.getText() } };
    };
}
TextInputWindow::TextInputHandler funcSubmitLoginPasswordEvent(bool isRegistration) {
    return [isRegistration](const TextInputWindow& win, const ClientSiteDocument& doc) {
        return Event{ windows::app::events::acceptLoginPassword, win.name(), windows::app::name, { (isRegistration ? windows::registration::name : windows::login::name) }};
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

double getCenteredLeft(const COORD consoleSize, const int width) {
    return ((double)consoleSize.X / 2 - width / 2) / consoleSize.X;
}