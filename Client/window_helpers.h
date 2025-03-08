#pragma once
#include "screen_buffers_builder.h"
#include "window_menu.h"
#include "window_info.h"
#include "window_text_input.h"
#include "window_text_editor.h"

// Specific window builders
ScrollableScreenBufferBuilder makeCreateDocWindowBuilder(const COORD& consoleSize);
ScrollableScreenBufferBuilder makeLoadDocWindowBuilder(const COORD& consoleSize);
ScrollableScreenBufferBuilder makeSearchWindowBuilder(const COORD& consoleSize);
ScrollableScreenBufferBuilder makeReplaceWindowBuilder(const COORD& consoleSize);
ScrollableScreenBufferBuilder makeTextEditorWindowBuilder(const COORD& consoleSize);
ScrollableScreenBufferBuilder makeInfoWindowBuilder(const COORD& consoleSize, const std::string& title);
ScrollableScreenBufferBuilder makeMenuWindowBuilder(const COORD& consoleSize, const std::string& title);
ScrollableScreenBufferBuilder makeGenericBuilder(const COORD& consoleSize, const std::string& title, const double left, const double top, const int width, const int height);

// Menu options
std::vector<Option> makeUnloggedMainMenuOptions();
std::vector<Option> makeLoggedMainMenuOptions();
std::vector<Option> makeConnectedToDocMainMenuOptions();

// TextInputHandlers
TextInputWindow::TextInputHandler funcSearchModifyEvent();
TextInputWindow::TextInputHandler funcSearchSubmitEvent();
TextInputWindow::TextInputHandler funcSearchDeleteEvent();
TextInputWindow::TextInputHandler funcReplaceSubmitEvent();
TextInputWindow::TextInputHandler funcJoinDocSubmitEvent();
TextInputWindow::TextInputHandler funcCreateDocSubmitEvent();
TextInputWindow::TextInputHandler funcSubmitLoginPasswordEvent(bool isRegistration);

std::string getHelpWindowText();
double getCenteredLeft(const COORD consoleSize, const int width);




