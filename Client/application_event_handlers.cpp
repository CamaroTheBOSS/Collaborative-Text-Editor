#include "application_event_handlers.h"
#include "application.h"

ApplicationEventHandlers::ApplicationEventHandlers() {
    handlers.try_emplace(windows::app::events::createDoc, &ApplicationEventHandlers::eventCreateDoc);
    handlers.try_emplace(windows::app::events::joinDoc, &ApplicationEventHandlers::eventJoinDoc);
    handlers.try_emplace(windows::app::events::exit, &ApplicationEventHandlers::eventMainMenuExitChosen);
    handlers.try_emplace(windows::app::events::createDocWindow, &ApplicationEventHandlers::eventMainMenuCreateChosen);
    handlers.try_emplace(windows::app::events::joinDocWindow, &ApplicationEventHandlers::eventMainMenuJoinChosen);
    handlers.try_emplace(windows::app::events::help, &ApplicationEventHandlers::eventMainMenuHelpChosen);
    handlers.try_emplace(windows::app::events::disconnect, &ApplicationEventHandlers::eventMainMenuDisconnectChosen);
    handlers.try_emplace(windows::app::events::showAcCode, &ApplicationEventHandlers::eventMainMenuShowAcCodeChosen);
}
bool ApplicationEventHandlers::processEvent(Application& app, const Event& pEvent) {
    auto it = handlers.find(pEvent.name);
    if (it == handlers.cend()) {
        return false;
    }
    Args args{ app, pEvent.params };
    (this->*it->second)(args);
    return true;
}
void ApplicationEventHandlers::eventMainMenuDisconnectChosen(const Args& input) {
    Application& app = input.app;
    app.disconnect();
    app.windowsManager.showWindow<InfoWindow>(
        makeInfoWindowBuilder(app.terminal.getScreenSize(), "Success"), "Disconnected successfuly"
    );
    app.windowsManager.destroyWindow("Main Menu", app.tcpClient);
}
void ApplicationEventHandlers::eventMainMenuCreateChosen(const Args& input) {
    input.app.windowsManager.showWindow<TextInputWindow>(
        makeCreateDocWindowBuilder(input.app.terminal.getScreenSize()), funcCreateDocSubmitEvent()
    );
}
void ApplicationEventHandlers::eventMainMenuJoinChosen(const Args& input) {
    input.app.windowsManager.showWindow<TextInputWindow>(
        makeLoadDocWindowBuilder(input.app.terminal.getScreenSize()), funcLoadDocSubmitEvent()
    );
}
void ApplicationEventHandlers::eventMainMenuExitChosen(const Args& input) {
    input.app.disconnect();
    exit(0);
}
void ApplicationEventHandlers::eventMainMenuHelpChosen(const Args& input) {
    input.app.windowsManager.showWindow<InfoWindow>(
        makeInfoWindowBuilder(input.app.terminal.getScreenSize(), "Help Control"), getHelpWindowText()
    );
}
void ApplicationEventHandlers::eventMainMenuShowAcCodeChosen(const Args& input) {
    input.app.windowsManager.showWindow<InfoWindow>(
        makeInfoWindowBuilder(input.app.terminal.getScreenSize(), "Access code"), input.app.repo.getAcCode()
    );
}

void ApplicationEventHandlers::eventCreateDoc(const Args& input) {
    bool success = joinCreateDocImpl(msg::Type::create, 1, input);
    if (success) {
        input.app.windowsManager.showWindow<InfoWindow>(
            makeInfoWindowBuilder(input.app.terminal.getScreenSize(), "Success"), "Access code for document: " + input.app.repo.getAcCode()
        );
    }
}
void ApplicationEventHandlers::eventJoinDoc(const Args& input) {
    bool success = joinCreateDocImpl(msg::Type::join, 1, input);
    if (success) {
        input.app.windowsManager.showWindow<InfoWindow>(
            makeInfoWindowBuilder(input.app.terminal.getScreenSize(), "Success"), "You connected successfuly"
        );
    }
}
bool ApplicationEventHandlers::joinCreateDocImpl(const msg::Type type, msg::OneByteInt version, const Args& input) {
    Application& app = input.app;
    if (input.args.empty()) {
        return false;
    }
    if (app.isConnected()) {
        app.windowsManager.showWindow<InfoWindow>(
            makeInfoWindowBuilder(app.terminal.getScreenSize(), "Failure"), "You are already connected to the document. Please disconnect first."
        );
        return false;
    }
    if (!app.connect(app.srvIp, app.srvPort)) {
        app.windowsManager.showWindow<InfoWindow>(
            makeInfoWindowBuilder(app.terminal.getScreenSize(), "Failure"), "Cannot connect to the server"
        );
        return false;
    }
    unsigned int socket = 0;
    app.tcpClient.sendMsg(type, version, socket, input.args[0]);
    bool connected = app.waitForDocument(std::chrono::milliseconds(500), 4);
    if (!connected) {
        app.windowsManager.showWindow<InfoWindow>(
            makeInfoWindowBuilder(app.terminal.getScreenSize(), "Failure"), "Timeout during document synchronization"
        );
        app.disconnect();
        return false;
    }
    std::string errorMsg = app.repo.getLastError();
    if (!errorMsg.empty()) {
        app.windowsManager.showWindow<InfoWindow>(
            makeInfoWindowBuilder(app.terminal.getScreenSize(), "Failure"), errorMsg
        );
        app.disconnect();
        return false;
    }
    app.windowsManager.destroyLastWindow(app.tcpClient);
    app.windowsManager.destroyWindow("Main Menu", app.tcpClient);
    return true;
}