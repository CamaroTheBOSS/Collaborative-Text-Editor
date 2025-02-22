#include "screen_builders.h"
#include "window_text_editor.h"
#include "window_search.h"
#include "window_replace.h"
#include "window_mainmenu.h"
#include "window_help.h"
#include "window_createdoc.h"
#include "window_loaddoc.h"

ScrollableScreenBufferBuilder makeMainMenuBuilder() {
    ScrollableScreenBufferBuilder builder;
    builder.setScrollHisteresis(0)
        .setTitle(MainMenuWindow::className)
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
ScrollableScreenBufferBuilder makeHelpWindowBuilder() {
    ScrollableScreenBufferBuilder builder;
    builder.setTitle("Help Control")
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