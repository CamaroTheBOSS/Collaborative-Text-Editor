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
    client(),
    terminal(),
    windowsManager(terminal.getScreenSize()),
    repo() {
    windowsManager.showWindow<TextEditorWindow>(makeTextEditorWindowBuilder(terminal.getScreenSize()));
    windowsManager.showWindow<MenuWindow>(makeMenuWindowBuilder(terminal.getScreenSize(), "Main Menu"), makeMainMenuOptions());
    eventHandlers.try_emplace(windows::app::events::createDoc, &Application::createDoc);
    eventHandlers.try_emplace(windows::app::events::loadDoc, &Application::loadDoc);
    eventHandlers.try_emplace(windows::app::events::exit, &Application::exitApp);
    eventHandlers.try_emplace(windows::app::events::createDocWindow, &Application::createDocWindow);
    eventHandlers.try_emplace(windows::app::events::loadDocWindow, &Application::loadDocWindow);
    eventHandlers.try_emplace(windows::app::events::help, &Application::helpWindow);
    eventHandlers.try_emplace(windows::app::events::disconnect, &Application::disconnectEvent);
    eventHandlers.try_emplace(windows::app::events::showAcCode, &Application::showAcCode);
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

void Application::disconnectEvent(const TCPClient& client, const std::vector<std::string>& args) {
    disconnect();
    windowsManager.showWindow<InfoWindow>(makeInfoWindowBuilder(terminal.getScreenSize(), "Success"), "Disconnected successfuly");
    windowsManager.destroyWindow("Main Menu", client);
}

void Application::createDoc(const TCPClient& client, const std::vector<std::string>& args) {
    bool success = loadCreateDoc(msg::Type::create, client, args);
    if (success) {
        windowsManager.showWindow<InfoWindow>(makeInfoWindowBuilder(terminal.getScreenSize(), "Success"), "Access code for document: " + repo.getAcCode());
    }
}
void Application::createDocWindow(const TCPClient& client, const std::vector<std::string>& args) {
    /*windowsManager.showTextInputWindow(makeCreateDocWindowBuilder(terminal.getScreenSize()),
        funcCreateDocSubmitEvent());*/
    windowsManager.showWindow<TextInputWindow>(makeCreateDocWindowBuilder(terminal.getScreenSize()), funcCreateDocSubmitEvent());
}
void Application::loadDoc(const TCPClient& client, const std::vector<std::string>& args) {
    bool success = loadCreateDoc(msg::Type::join, client, args);
    if (success) {
        windowsManager.showWindow<InfoWindow>(makeInfoWindowBuilder(terminal.getScreenSize(), "Success"), "You connected successfuly");
    }
}

void Application::loadDocWindow(const TCPClient& client, const std::vector<std::string>& args) {
    windowsManager.showWindow<TextInputWindow>(makeLoadDocWindowBuilder(terminal.getScreenSize()), funcLoadDocSubmitEvent());
}

bool Application::loadCreateDoc(const msg::Type type, const TCPClient& client, const std::vector<std::string>& args) {
    if (args.empty()) {
        return false;
    }
    if (isConnected()) {
        windowsManager.showWindow<InfoWindow>(makeInfoWindowBuilder(terminal.getScreenSize(), "Failure"), "You are already connected to the document. Please disconnect first.");
        return false;
    }
    if (!connect(srvIp, srvPort)) {
        windowsManager.showWindow<InfoWindow>(makeInfoWindowBuilder(terminal.getScreenSize(), "Failure"), "Cannot connect to the server");
        return false;
    }
    unsigned int socket = 0;
    client.sendMsg(type, version, socket, args[0]);
    bool connected = waitForDocument(std::chrono::milliseconds(500), 4);
    if (!connected) {
        windowsManager.showWindow<InfoWindow>(makeInfoWindowBuilder(terminal.getScreenSize(), "Failure"), "Timeout during document synchronization");
        disconnect();
        return false;
    }
    std::string errorMsg = repo.getLastError();
    if (!errorMsg.empty()) {
        windowsManager.showWindow<InfoWindow>(makeInfoWindowBuilder(terminal.getScreenSize(), "Failure"), errorMsg);
        disconnect();
        return false;
    }
    windowsManager.destroyLastWindow(client);
    windowsManager.destroyWindow("Main Menu", client);
    return true;
}

void Application::exitApp(const TCPClient& client, const std::vector<std::string>& args) {
    disconnect();
    exit(0);
}

void Application::helpWindow(const TCPClient& client, const std::vector<std::string>& args) {
    windowsManager.showWindow<InfoWindow>(makeInfoWindowBuilder(terminal.getScreenSize(), "Help Control"), getHelpWindowText());
}

void Application::showAcCode(const TCPClient& client, const std::vector<std::string>& args) {
    windowsManager.showWindow<InfoWindow>(makeInfoWindowBuilder(terminal.getScreenSize(), "Access code"), repo.getAcCode());
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
            return true;
        }
    }
    logger.logDebug(currTry, "Requesting document from the server failed!\n");
    return false;
}

void Application::render() {
    terminal.render(windowsManager.getWindows());
}
