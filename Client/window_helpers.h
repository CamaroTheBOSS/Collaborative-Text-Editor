#pragma once
#include "screen_buffers_builder.h"
#include "window_menu.h"

ScrollableScreenBufferBuilder makeCreateDocWindowBuilder();
ScrollableScreenBufferBuilder makeLoadDocWindowBuilder();
ScrollableScreenBufferBuilder makeSearchWindowBuilder();
ScrollableScreenBufferBuilder makeReplaceWindowBuilder();
ScrollableScreenBufferBuilder makeTextEditorWindowBuilder();
ScrollableScreenBufferBuilder makeInfoWindowBuilder(const std::string& title);
ScrollableScreenBufferBuilder makeMenuWindowBuilder(const std::string& title);

std::vector<Option> makeMainMenuOptions();
std::vector<Option> makeLoggedMainMenuOptions();

std::string getHelpWindowText();




