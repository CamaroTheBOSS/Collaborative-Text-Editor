module;

#include <vector>
#include <string>
#include "pos_helpers.h"

export module window.helpers;
import screen.buffers.builder;
import window.menu;
import window.info;
import window.text_input;
import window.text_editor;

// Specific window builders
export ScrollableScreenBufferBuilder makeLoadDocWindowBuilder(const COORD& consoleSize);
export ScrollableScreenBufferBuilder makeSearchWindowBuilder(const COORD& consoleSize);
export ScrollableScreenBufferBuilder makeReplaceWindowBuilder(const COORD& consoleSize);
export ScrollableScreenBufferBuilder makeTextEditorWindowBuilder(const COORD& consoleSize);
export ScrollableScreenBufferBuilder makeCreateDocWindowBuilder(const COORD& consoleSize);
export ScrollableScreenBufferBuilder makeInfoWindowBuilder(const COORD& consoleSize, const std::string& title);
export ScrollableScreenBufferBuilder makeMenuWindowBuilder(const COORD& consoleSize, const std::string& title);
export ScrollableScreenBufferBuilder makeGenericBuilder(const COORD& consoleSize, const std::string& title, const double left, const double top, const int width, const int height);

// Menu options
export std::vector<Option> makeUnloggedMainMenuOptions();
export std::vector<Option> makeLoggedMainMenuOptions();
export std::vector<Option> makeConnectedToDocMainMenuOptions();

// TextInputHandlers
export TextInputWindow::TextInputHandler funcSearchModifyEvent();
export TextInputWindow::TextInputHandler funcSearchSubmitEvent();
export TextInputWindow::TextInputHandler funcSearchDeleteEvent();
export TextInputWindow::TextInputHandler funcReplaceSubmitEvent();
export TextInputWindow::TextInputHandler funcJoinDocSubmitEvent();
export TextInputWindow::TextInputHandler funcCreateDocSubmitEvent();
export TextInputWindow::TextInputHandler funcSubmitLoginPasswordEvent(bool isRegistration);

export std::string getHelpWindowText();
export double getCenteredLeft(const COORD consoleSize, const int width);


