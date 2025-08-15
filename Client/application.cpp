#include <chrono>
#include <thread>
#include <assert.h>
#include <array>

#include "application.h"
#include "logging.h"
#include "keypack_defs.h"

import events;
import window.text_editor;
import window.text_input;
import window.text_input.obsfucated;
import window.info;
import window.menu;
import window.helpers;
import validator;

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
    windowsManager.showWindow<MenuWindow>(makeMenuWindowBuilder(terminal.getScreenSize(), windows::mainmenu::name), getMainMenuOptions());
    /*eventHandlers = {
        {windows::app::events::createDoc, &eventCreateDoc},
        {windows::app::events::joinDoc, &eventJoinDoc},
        {windows::app::events::exit, &eventMainMenuExitChosen},
        {windows::app::events::createDocWindow, &eventMainMenuCreateChosen},
        {windows::app::events::joinDocWindow, &eventMainMenuJoinChosen},
        {windows::app::events::loadDocWindow, &eventMainMenuLoadChosen},
        {windows::app::events::loadItemClicked, &eventLoadItemClicked},
        {windows::app::events::loadItemAccepted, &eventLoadItemAccepted},
        {windows::app::events::loadItemDeleted, &eventLoadItemDeleted},
        {windows::app::events::help, &eventMainMenuHelpChosen},
        {windows::app::events::disconnect, &eventMainMenuDisconnectChosen},
        {windows::app::events::showAcCode, &eventMainMenuShowAcCodeChosen},
        {windows::app::events::showLoginWindow, &eventMainMenuLoginRegisterChosen},
        {windows::app::events::acceptLoginPassword, &eventLoginPasswordAccepted},
    };*/
}

bool Application::connect(const std::string& ip, const int port) {
    return tcpClient.connectServer(ip, port);
}

bool Application::disconnect() {
    bool disconnected = tcpClient.disconnect();
    if (disconnected) {
        repo.cleanAcCode();
        repo.cleanAuthToken();
        windowsManager.getTextEditor()->clearContent();
        windowsManager.destroyWindow(windows::mainmenu::name, tcpClient);
    }
    return disconnected;
}

bool Application::isConnected() const {
    return tcpClient.isConnected();
}

bool Application::isConnectedToDoc() const {
    return isConnected() && !repo.getAcCode().empty();
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
        windowsManager.showWindow<MenuWindow>(
            makeMenuWindowBuilder(terminal.getScreenSize(), windows::mainmenu::name), getMainMenuOptions()
        );
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
        processEvent(pEvent);
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

bool Application::waitForResponse(const msg::Type desiredType, const std::chrono::milliseconds& timeout, const int tries) {
    int currTry = 0;
    while (currTry < tries) {
        msg::Buffer msgBuffer = tcpClient.getNextMsg();
        if (msgBuffer.empty()) {
            currTry++;
            std::this_thread::sleep_for(timeout);
            continue;
        }
        msg::Type msgType;
        msg::parse(msgBuffer, 0, msgType);
        repo.processMsg(windowsManager.getTextEditor()->getDocMutable(), msgBuffer);
        if (msgType == desiredType) {
            return true;
        }
    }
    logger.logDebug(currTry, "Waiting for response from the server timed out!\n");
    return false;
}

void Application::render() {
    terminal.render(windowsManager.getWindows());
}

std::vector<Option> Application::getMainMenuOptions() const {
    if (isLogged() && isConnectedToDoc()) {
        return makeConnectedToDocMainMenuOptions();
    }
    if (isLogged()) {
        return makeLoggedMainMenuOptions();
    }
    return makeUnloggedMainMenuOptions();
}

bool Application::processEvent(const Event& pEvent) {
    auto it = eventHandlers.find(pEvent.name);
    if (it == eventHandlers.cend()) {
        return false;
    }
    (*it->second)(pEvent);
    return true;
}

void Application::eventMainMenuLoginRegisterChosen(const Event& pEvent) {
    assert(pEvent.params.size() > 0);
    auto screenSize = terminal.getScreenSize();
    int width = 15;
    double left = getCenteredLeft(screenSize, width);
    windowsManager.showWindow<ObsfucatedTextInputWindow>(
        makeGenericBuilder(screenSize, windows::password::name, left, 0.35, width, 1),
        funcSubmitLoginPasswordEvent(pEvent.params[0] == windows::registration::name)
    );
    windowsManager.showWindow<TextInputWindow>(
        makeGenericBuilder(screenSize, windows::login::name, left, 0.2, width, 1),
        funcSubmitLoginPasswordEvent(pEvent.params[0] == windows::registration::name)
    );
}

void Application::eventLoginPasswordAccepted(const Event& pEvent) {
    auto& allWindows = windowsManager.getWindows();
    struct WinWithComplementaryName {
        WindowsIt window;
        std::string complementaryName;
    };
    std::array<WinWithComplementaryName, 2> checkedWindows = {
        WinWithComplementaryName{windowsManager.findWindow(windows::login::name), windows::password::name},
        WinWithComplementaryName{windowsManager.findWindow(windows::password::name), windows::login::name}
    };
    int width = 20;
    auto screenSize = terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    for (const auto& winAndName : checkedWindows) {
        if (winAndName.window == allWindows.cend()) {
            windowsManager.destroyWindow(winAndName.complementaryName, tcpClient);
            windowsManager.showWindow<InfoWindow>(
                makeGenericBuilder(screenSize, "Failed", left, 0.4, width, 3), "Cannot find window " + winAndName.window->get()->name()
            );
            return;
        }
        if (!validateTextInputWindow(winAndName.window)) {
            return;
        }
    }
    assert(pEvent.params.size() == 1);
    auto type = pEvent.params[0] == windows::registration::name ? msg::Type::registration : msg::Type::login;
    msg::OneByteInt version = 1;
    if (!validateConnection()) {
        return;
    }
    tcpClient.sendMsg(type, version, checkedWindows[0].window->get()->getDoc().getText(), checkedWindows[1].window->get()->getDoc().getText());
    if (!waitForResponseAndProccessIt(type)) {
        return;
    }
    windowsManager.destroyWindow(windows::login::name, tcpClient);
    windowsManager.destroyWindow(windows::password::name, tcpClient);
    windowsManager.destroyWindow(windows::mainmenu::name, tcpClient);
    windowsManager.showWindow<MenuWindow>(
        makeMenuWindowBuilder(screenSize, windows::mainmenu::name), getMainMenuOptions()
    );
    windowsManager.showWindow<InfoWindow>(
        makeGenericBuilder(screenSize, "Success", left, 0.4, width, 1), pEvent.params[0] + " successful"
    );

    std::string authToken = repo.getAuthToken();
    if (authToken.empty()) {
        return;
    }
    auto textEditor = dynamic_cast<TextEditorWindow*>(windowsManager.getTextEditor().get());
    assert(textEditor != nullptr);
    textEditor->setAuthToken(authToken);
}

void Application::eventMainMenuDisconnectChosen(const Event& pEvent) {
    int width = 25;
    auto screenSize = terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    disconnect();
    windowsManager.showWindow<InfoWindow>(
        makeGenericBuilder(screenSize, "Success", left, 0.4, width, 1), "Logged out successfuly"
    );
    windowsManager.destroyWindow(windows::mainmenu::name, tcpClient);
}

void Application::eventMainMenuCreateChosen(const Event& pEvent) {
    windowsManager.showWindow<TextInputWindow>(
        makeCreateDocWindowBuilder(terminal.getScreenSize()), funcCreateDocSubmitEvent()
    );
}

void Application::eventMainMenuJoinChosen(const Event& pEvent) {
    windowsManager.showWindow<TextInputWindow>(
        makeLoadDocWindowBuilder(terminal.getScreenSize()), funcJoinDocSubmitEvent()
    );
}

void Application::eventMainMenuLoadChosen(const Event& pEvent) {
    msg::OneByteInt version = 1;
    tcpClient.sendMsg(msg::Type::getDocNames, version, repo.getAuthToken());
    if (!waitForResponseAndProccessIt(msg::Type::getDocNames)) {
        windowsManager.destroyWindow(pEvent.src, tcpClient);
        return;
    }
    std::vector<Option> options;
    for (const auto& name : repo.getFetchedDocNames()) {
        options.emplace_back(Option{ name, [&](MenuWindow& obj) { return Event{ windows::app::events::loadItemClicked, obj.name(), windows::app::name, { name }}; } });
    }
    if (options.size() > 0) {
        windowsManager.showWindow<MenuWindow>(
            makeMenuWindowBuilder(terminal.getScreenSize(), "Choose document"), std::move(options)
        );
    }
    else {
        int width = 25;
        auto screenSize = terminal.getScreenSize();
        double left = getCenteredLeft(screenSize, width);
        windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(terminal.getScreenSize(), "Choose document", left, 0.4, width, 1), "You don't have any documents"
        );
    }

}

void Application::eventMainMenuExitChosen(const Event& pEvent) {
    disconnect();
    exit(0);
}

void Application::eventMainMenuHelpChosen(const Event& pEvent) {
    windowsManager.showWindow<InfoWindow>(
        makeInfoWindowBuilder(terminal.getScreenSize(), "Help Control"), getHelpWindowText()
    );
}

void Application::eventMainMenuShowAcCodeChosen(const Event& pEvent) {
    int width = 15;
    auto screenSize = terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    windowsManager.showWindow<InfoWindow>(
        makeGenericBuilder(screenSize, "Access code", left, 0.4, width, 1), repo.getAcCode()
    );
}

void Application::eventLoadItemClicked(const Event& pEvent) {
    assert(pEvent.params.size() > 0);
    windowsManager.showWindow<MenuWindow>(
        makeMenuWindowBuilder(terminal.getScreenSize(), "Manage document"), std::vector<Option>{
        Option{ "Load", [filename = pEvent.params[0]](MenuWindow& obj) { return Event{ windows::app::events::loadItemAccepted, obj.name(), windows::app::name, { filename }}; } },
            Option{ "Delete", [filename = pEvent.params[0]](MenuWindow& obj) { return Event{ windows::app::events::loadItemDeleted, obj.name(), windows::app::name, { filename } }; } }
    });
}
void Application::eventLoadItemAccepted(const Event& pEvent) {
    bool success = joinCreateDocImpl(msg::Type::load, 1, pEvent);
    if (success) {
        int width = 26;
        auto screenSize = terminal.getScreenSize();
        double left = getCenteredLeft(screenSize, width);
        windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(screenSize, "Success", left, 0.4, width, 1),
            "Access code for document: " + repo.getAcCode()
        );
        windowsManager.destroyWindow(pEvent.src, tcpClient);
        windowsManager.destroyWindow("Choose document", tcpClient);
    }
}
void Application::eventLoadItemDeleted(const Event& pEvent) {
    assert(pEvent.params.size() > 0);
    msg::OneByteInt version = 1;
    tcpClient.sendMsg(msg::Type::delDoc, version, repo.getAuthToken(), pEvent.params[0]);
    if (!waitForResponseAndProccessIt(msg::Type::delDoc)) {
        windowsManager.destroyWindow(pEvent.src, tcpClient);
        return;
    }
    int width = 25;
    auto screenSize = terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    windowsManager.showWindow<InfoWindow>(
        makeGenericBuilder(screenSize, "Success", left, 0.4, width, 1),
        "Document deleted successfuly"
    );
    windowsManager.destroyWindow(pEvent.src, tcpClient);
    windowsManager.destroyWindow("Choose document", tcpClient);
}

void Application::eventCreateDoc(const Event& pEvent) {
    bool success = joinCreateDocImpl(msg::Type::create, 1, pEvent);
    if (success) {
        int width = 26;
        auto screenSize = terminal.getScreenSize();
        double left = getCenteredLeft(screenSize, width);
        windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(screenSize, "Success", left, 0.4, width, 1),
            "Access code for document: " + repo.getAcCode()
        );
    }
}

void Application::eventJoinDoc(const Event& pEvent) {
    bool success = joinCreateDocImpl(msg::Type::join, 1, pEvent);
    if (success) {
        int width = 25;
        auto screenSize = terminal.getScreenSize();
        double left = getCenteredLeft(screenSize, width);
        windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(screenSize, "Success", left, 0.4, width, 1),
            "You connected successfuly"
        );
    }
}

bool Application::joinCreateDocImpl(const msg::Type type, msg::OneByteInt version, const Event& pEvent) {
    auto& params = pEvent.params;
    assert(params.size() > 0);
    int width = 25;
    auto screenSize = terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    if (!isLogged()) {
        return false;
    }
    if (isConnectedToDoc()) {
        windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(terminal.getScreenSize(), "Failure", left, 0.4, width, 2), "You are already connected to the document. Please disconnect first."
        );
        return false;
    }
    if (!validateConnection()) {
        return false;
    }
    if (type != msg::Type::load && !validateTextInputWindow(windowsManager.findWindow(pEvent.src))) {
        return false;
    }
    unsigned int socket = 0;
    tcpClient.sendMsg(type, version, socket, params[0]);
    if (!waitForResponseAndProccessIt(type)) {
        windowsManager.destroyWindow(pEvent.src, tcpClient);
        return false;
    }
    windowsManager.destroyLastWindow(tcpClient);
    windowsManager.destroyWindow(windows::mainmenu::name, tcpClient);
    return true;
}

bool Application::waitForResponseAndProccessIt(const msg::Type type) {
    bool success = waitForResponse(type, std::chrono::milliseconds(1), 1500);
    int width = 25;
    auto screenSize = terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    if (!success) {
        windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(terminal.getScreenSize(), "Failure", left, 0.4, width, 2), "No response from the server"
        );
        disconnect();
        return false;
    }
    std::string errorMsg = repo.getLastError();
    if (!errorMsg.empty()) {
        windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(terminal.getScreenSize(), "Failure", left, 0.4, width, 2), errorMsg
        );
        disconnect();
        return false;
    }

    return true;
}

bool Application::validateTextInputWindow(const WindowsIt& window) {
    auto txt = window->get()->getDoc().getText();
    auto msg = Validator::validateString(txt);
    if (!msg.empty()) {
        int width = 25;
        auto screenSize = terminal.getScreenSize();
        double left = getCenteredLeft(screenSize, width);
        auto winMsg = window->get()->name() + " doesn't meet required criteria '" + msg + "'";
        windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(screenSize, "Failed", left, 0.4, width, 3), winMsg
        );
        return false;
    }
    return true;
}

bool Application::validateConnection() {
    int width = 15;
    auto screenSize = terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    if (!connect(srvIp, srvPort)) {
        windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(terminal.getScreenSize(), "Failure", left, 0.4, width, 2), "Cannot connect to the server"
        );
        return false;
    }
    return true;
}
