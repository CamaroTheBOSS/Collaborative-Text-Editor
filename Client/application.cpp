#include <chrono>
#include <thread>

#include "window_text_editor.h"
#include "window_search.h"
#include "window_replace.h"
#include "application.h"
#include "logging.h"

using namespace client;
constexpr msg::OneByteInt version = 1;

Application::Application() :
    client(),
    terminal(),
    windowsManager(),
    repo() {
    windowsManager.showWindow<TextEditorWindow>(terminal.getScreenSize());
    windowsManager.showWindow<MainMenuWindow>(terminal.getScreenSize());
    eventHandlers.try_emplace(windows::app::events::createDoc, &Application::createDoc);
    eventHandlers.try_emplace(windows::app::events::loadDoc, &Application::loadDoc);
    eventHandlers.try_emplace(windows::app::events::exit, &Application::exitApp);
    eventHandlers.try_emplace(windows::app::events::createDocWindow, &Application::createDocWindow);
    eventHandlers.try_emplace(windows::app::events::loadDocWindow, &Application::loadDocWindow);
    eventHandlers.try_emplace(windows::app::events::help, &Application::helpWindow);
}

bool Application::connect(const std::string& ip, const int port) {
    return client.connectServer(ip, port);
}

bool Application::disconnect() {
    return client.disconnect();
}

bool Application::isConnected() const {
    return client.isConnected();
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
        window->processChar(client, key, terminal.getClipboardData());
        return true;
    case CTRL_C:
        terminal.setClipboardData(window->getDoc().getSelectedText());
        return false;
    case CTRL_X:
        terminal.setClipboardData(window->getDoc().getSelectedText());
        window->processChar(client, key);
        return true;
    case CTRL_F:
    case F3:
        windowsManager.showWindow<SearchWindow>(terminal.getScreenSize());
        return true;
    case CTRL_R:
        if (key.shiftPressed) {
            windowsManager.showWindow<SearchWindow>(terminal.getScreenSize());
            windowsManager.showWindow<ReplaceWindow>(terminal.getScreenSize());
        }
        return true;
    case CTRL_Q:
        windowsManager.showWindow<MainMenuWindow>(terminal.getScreenSize());
        return true;
    case ESC:
        windowsManager.destroyLastWindow(client);
        return true;
    }
    Event pEvent = window->processChar(client, key);
    if (pEvent.empty()) {
        return true;
    }
    else if (pEvent.target == "App") {
        processEvent(pEvent);
    }
    else {
        windowsManager.processEvent(client, pEvent);
    }
    return true;
}

bool Application::processEvent(const Event& pEvent) {
    auto it = eventHandlers.find(pEvent.name);
    if (it == eventHandlers.cend()) {
        return false;
    }
    (this->*it->second)(client, pEvent.params);
    return true;
}

void Application::createDoc(const TCPClient& client, const std::vector<std::string>& args) {
    if (docRequested || args.empty()) {
        return;
    }
    unsigned int socket = 0;
    client.sendMsg(msg::Type::create, version, socket, args[0]);
    waitForDocument(std::chrono::milliseconds(500), 4000);
    windowsManager.destroyLastWindow(client);
    windowsManager.destroyWindow(MainMenuWindow::className, client);
    std::string errorMsg = repo.getLastError();
    if (!errorMsg.empty()) {
        windowsManager.showInfoWindow(terminal.getScreenSize(), "Error", errorMsg);
        return;
    }
    windowsManager.showInfoWindow(terminal.getScreenSize(), "Success", "Access code for document: " + repo.getAcCode());
}
void Application::createDocWindow(const TCPClient& client, const std::vector<std::string>& args) {
    windowsManager.showWindow<CreateDocWindow>(terminal.getScreenSize());
}
void Application::loadDoc(const TCPClient& client, const std::vector<std::string>& args) {
    if (docRequested || args.empty()) {
        return;
    }
    unsigned int socket = 0;
    client.sendMsg(msg::Type::join, version, socket, args[0]);
    waitForDocument(std::chrono::milliseconds(500), 4000);
    windowsManager.destroyLastWindow(client);
    windowsManager.destroyWindow(MainMenuWindow::className, client);
    std::string errorMsg = repo.getLastError();
    if (!errorMsg.empty()) {
        windowsManager.showInfoWindow(terminal.getScreenSize(), "Error", errorMsg);
        return;
    }
    windowsManager.showInfoWindow(terminal.getScreenSize(), "Success", "You connected successfuly");
}

void Application::loadDocWindow(const TCPClient& client, const std::vector<std::string>& args) {
    windowsManager.showWindow<LoadDocWindow>(terminal.getScreenSize());
}
void Application::exitApp(const TCPClient& client, const std::vector<std::string>& args) {
    exit(0);
}

void Application::helpWindow(const TCPClient& client, const std::vector<std::string>& args) {
    windowsManager.showInfoWindow(terminal.getScreenSize(), "Help Control", 
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
        "C++ is cool\n"
    );
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
        msg::Buffer msgBuffer = client.getNextMsg();
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
        msg::Buffer msgBuffer = client.getNextMsg();
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
            docRequested = true;
            return true;
        }
    }
    logger.logDebug(currTry, "Requesting document from the server failed!\n");
    return false;
}

void Application::render() {
    terminal.render(windowsManager.getWindows());
}
