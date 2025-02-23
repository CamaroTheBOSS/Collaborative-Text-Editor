#pragma once
#include "screen_buffers_builder.h"

ScrollableScreenBufferBuilder makeMainMenuBuilder();
ScrollableScreenBufferBuilder makeCreateDocWindowBuilder();
ScrollableScreenBufferBuilder makeLoadDocWindowBuilder();
ScrollableScreenBufferBuilder makeSearchWindowBuilder();
ScrollableScreenBufferBuilder makeReplaceWindowBuilder();
ScrollableScreenBufferBuilder makeTextEditorWindowBuilder();
ScrollableScreenBufferBuilder makeInfoWindowBuilder(const std::string& title);
