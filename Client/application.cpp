#include <chrono>
#include <thread>

#include "window_text_editor.h"
#include "application.h"
#include "logging.h"
#include "window_helpers.h"

using namespace client;
constexpr msg::OneByteInt version = 1;

Application::Application(const std::string& ip, const int port) :
    srvIp(ip),
    srvPort(port),
    tcpClient(),
    terminal(),
    windowsManager(terminal.getScreenSize()),
    repo() {
    windowsManager.showWindow<TextEditorWindow>(makeTextEditorWindowBuilder(terminal.getScreenSize()));
    windowsManager.showWindow<MenuWindow>(makeMenuWindowBuilder(terminal.getScreenSize(), "Main Menu"), makeUnloggedMainMenuOptions());
}

bool Application::connect(const std::string& ip, const int port) {
    return tcpClient.connectServer(ip, port);
}

bool Application::disconnect() {
    return tcpClient.disconnect();
}

bool Application::isConnected() const {
    return tcpClient.isConnected();
}

bool Application::isLogged() const {
    return !repo.getAuthToken().empty();
}

KeyPack Application::readChar() const {
    return terminal.readChar();
}

bool Application::processChar(const KeyPack& key) {
    auto& window = windowsManager.getFocusedWindow();
    switch (key.keyCode) {
    case CTRL_ARROW_DOWN:
        windowsManager.changeFocusDown();
        return true;
    case CTRL_ARROW_UP:
        windowsManager.changeFocusUp();
        return true;
    case CTRL_ARROW_LEFT:
        windowsManager.changeFocusLeft();
        return true;
    case CTRL_ARROW_RIGHT:
        windowsManager.changeFocusRight();
        return true;
    case CTRL_V:
        window->processChar(tcpClient, key, terminal.getClipboardData());
        return true;
    case CTRL_C:
        terminal.setClipboardData(window->getDoc().getSelectedText());
        return false;
    case CTRL_X:
        terminal.setClipboardData(window->getDoc().getSelectedText());
        window->processChar(tcpClient, key);
        return true;
    case CTRL_F:
    case F3:
        windowsManager.showWindow<TextInputWindow>(makeSearchWindowBuilder(terminal.getScreenSize()), 
            funcSearchSubmitEvent(), funcSearchModifyEvent(), funcSearchDeleteEvent());
        return true;
    case CTRL_R:
        if (key.shiftPressed) {
            windowsManager.showWindow<TextInputWindow>(makeSearchWindowBuilder(terminal.getScreenSize()), 
                funcSearchSubmitEvent(), funcSearchModifyEvent(), funcSearchDeleteEvent());
            windowsManager.showWindow<TextInputWindow>(makeReplaceWindowBuilder(terminal.getScreenSize()), funcReplaceSubmitEvent());
        }
        return true;
    case CTRL_Q:
        windowsManager.showWindow<MenuWindow>(makeMenuWindowBuilder(terminal.getScreenSize(), "Main Menu"),
            isConnected() ? makeLoggedMainMenuOptions() : makeMainMenuOptions());
        return true;
    case ESC:
        windowsManager.destroyLastWindow(tcpClient);
        return true;
    }
    Event pEvent = window->processChar(tcpClient, key);
    if (pEvent.empty()) {
        return true;
    }
    else if (pEvent.target == windows::app::name) {
        eventHandler.processEvent(*this, pEvent);
    }
    else {
        windowsManager.processEvent(tcpClient, pEvent);
    }
    return true;
}

bool Application::checkBufferWasResized() {
    bool screenResized = terminal.resizeScreenBufferIfNeeded();
    if (screenResized) {
        COORD newConsoleSize = terminal.getScreenSize();
        for (const auto& window : windowsManager.getWindows()) {
            window->updateConsoleSize({ newConsoleSize.X, newConsoleSize.Y });
        }
    }
    return screenResized;
}

bool Application::checkIncomingMessages() {
    int needRender = 0;
    while (true) {
        msg::Buffer msgBuffer = tcpClient.getNextMsg();
        if (msgBuffer.empty()) {
            break;
        }
        needRender += repo.processMsg(windowsManager.getTextEditor()->getDocMutable(), msgBuffer);
    }
    needRender += checkBufferWasResized();
    return needRender;
}

bool Application::waitForDocument(const std::chrono::milliseconds& timeout, const int tries) {
    int currTry = 0;
    while (currTry < tries) {
        msg::Buffer msgBuffer = tcpClient.getNextMsg();
        if (msgBuffer.empty()) {
            currTry++;
            logger.logDebug(currTry, " try to request document failed");
            std::this_thread::sleep_for(timeout);
            continue;
        }
        msg::Type msgType;
        msg::parse(msgBuffer, 0, msgType);
        repo.processMsg(windowsManager.getTextEditor()->getDocMutable(), msgBuffer);
        if (msgType == msg::Type::create || msgType == msg::Type::join) {
            return true;
        }
    }
    logger.logDebug(currTry, "Requesting document from the server failed!\n");
    return false;
}

void Application::render() {
    terminal.render(windowsManager.getWindows());
}
