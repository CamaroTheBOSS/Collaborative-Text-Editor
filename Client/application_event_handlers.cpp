#include "application_event_handlers.h"
#include "application.h"
#include "validator.h"

#include <array>

ApplicationEventHandlers::ApplicationEventHandlers():
    handlers({
        {windows::app::events::createDoc, &ApplicationEventHandlers::eventCreateDoc},
        {windows::app::events::joinDoc, &ApplicationEventHandlers::eventJoinDoc},
        {windows::app::events::exit, &ApplicationEventHandlers::eventMainMenuExitChosen},
        {windows::app::events::createDocWindow, &ApplicationEventHandlers::eventMainMenuCreateChosen},
        {windows::app::events::joinDocWindow, &ApplicationEventHandlers::eventMainMenuJoinChosen},
        {windows::app::events::help, &ApplicationEventHandlers::eventMainMenuHelpChosen},
        {windows::app::events::disconnect, &ApplicationEventHandlers::eventMainMenuDisconnectChosen},
        {windows::app::events::showAcCode, &ApplicationEventHandlers::eventMainMenuShowAcCodeChosen},
        {windows::app::events::showLoginWindow, &ApplicationEventHandlers::eventMainMenuLoginRegisterChosen},
        {windows::app::events::acceptLoginPassword, &ApplicationEventHandlers::eventLoginPasswordAccepted},
        }) {}

bool ApplicationEventHandlers::processEvent(Application& app, const Event& pEvent) {
    auto it = handlers.find(pEvent.name);
    if (it == handlers.cend()) {
        return false;
    }
    (this->*it->second)(app, pEvent);
    return true;
}

void ApplicationEventHandlers::eventMainMenuLoginRegisterChosen(Application& app, const Event& pEvent) {
    assert(pEvent.params.size() > 0);
    auto screenSize = app.terminal.getScreenSize();
    int width = 15;
    double left = ((double)screenSize.X / 2 - 7) / screenSize.X;
    app.windowsManager.showWindow<TextInputWindow>(
        makeTextInputBuilder(app.terminal.getScreenSize(), 
        windows::login::name, left, 0.2, width, 1),
        funcSubmitLoginPasswordEvent(pEvent.params[0] == windows::registration::name)
    );
    app.windowsManager.showWindow<TextInputWindow>(
        makeTextInputBuilder(app.terminal.getScreenSize(), 
        windows::password::name, left, 0.35, width, 1),
        funcSubmitLoginPasswordEvent(pEvent.params[0] == windows::registration::name)
    );
}

void ApplicationEventHandlers::eventLoginPasswordAccepted(Application& app, const Event& pEvent) {
    auto& allWindows = app.windowsManager.getWindows();
    struct WinWithComplementaryName {
        WindowsIt window;
        std::string complementaryName;
    };
    std::array<WinWithComplementaryName, 2> checkedWindows = {
        WinWithComplementaryName{app.windowsManager.findWindow(windows::login::name), windows::password::name},
        WinWithComplementaryName{app.windowsManager.findWindow(windows::password::name), windows::login::name}
    };
    for (const auto& winAndName : checkedWindows) {
        if (winAndName.window == allWindows.cend()) {
            app.windowsManager.destroyWindow(winAndName.complementaryName, app.tcpClient);
            app.windowsManager.showWindow<InfoWindow>(
                makeInfoWindowBuilder(app.terminal.getScreenSize(), "Failed"), "Cannot find window " + winAndName.window->get()->name()
            );
            return;
        }
        auto txt = winAndName.window->get()->getDoc().getText();
        auto msg = Validator::validateString(txt);
        if (!msg.empty()) {
            auto winMsg = winAndName.window->get()->name() + " doesn't meet required criteria '" + msg + "'";
            app.windowsManager.showWindow<InfoWindow>(
                makeInfoWindowBuilder(app.terminal.getScreenSize(), "Failed"), winMsg
            );
            return;
        }
    }
    // LOGING/REGISTERING IMPL
}

void ApplicationEventHandlers::eventLogout(Application& app, const Event& pEvent) {

}

void ApplicationEventHandlers::eventMainMenuDisconnectChosen(Application& app, const Event& pEvent) {
    app.disconnect();
    app.windowsManager.showWindow<InfoWindow>(
        makeInfoWindowBuilder(app.terminal.getScreenSize(), "Success"), "Disconnected successfuly"
    );
    app.windowsManager.destroyWindow("Main Menu", app.tcpClient);
}

void ApplicationEventHandlers::eventMainMenuCreateChosen(Application& app, const Event& pEvent) {
    app.windowsManager.showWindow<TextInputWindow>(
        makeCreateDocWindowBuilder(app.terminal.getScreenSize()), funcCreateDocSubmitEvent()
    );
}

void ApplicationEventHandlers::eventMainMenuJoinChosen(Application& app, const Event& pEvent) {
    app.windowsManager.showWindow<TextInputWindow>(
        makeLoadDocWindowBuilder(app.terminal.getScreenSize()), funcLoadDocSubmitEvent()
    );
}

void ApplicationEventHandlers::eventMainMenuExitChosen(Application& app, const Event& pEvent) {
    app.disconnect();
    exit(0);
}

void ApplicationEventHandlers::eventMainMenuHelpChosen(Application& app, const Event& pEvent) {
    app.windowsManager.showWindow<InfoWindow>(
        makeInfoWindowBuilder(app.terminal.getScreenSize(), "Help Control"), getHelpWindowText()
    );
}

void ApplicationEventHandlers::eventMainMenuShowAcCodeChosen(Application& app, const Event& pEvent) {
    app.windowsManager.showWindow<InfoWindow>(
        makeInfoWindowBuilder(app.terminal.getScreenSize(), "Access code"), app.repo.getAcCode()
    );
}

void ApplicationEventHandlers::eventCreateDoc(Application& app, const Event& pEvent) {
    bool success = joinCreateDocImpl(msg::Type::create, 1, app, pEvent);
    if (success) {
        app.windowsManager.showWindow<InfoWindow>(
            makeInfoWindowBuilder(app.terminal.getScreenSize(), "Success"), "Access code for document: " + app.repo.getAcCode()
        );
    }
}

void ApplicationEventHandlers::eventJoinDoc(Application& app, const Event& pEvent) {
    bool success = joinCreateDocImpl(msg::Type::join, 1, app, pEvent);
    if (success) {
        app.windowsManager.showWindow<InfoWindow>(
            makeInfoWindowBuilder(app.terminal.getScreenSize(), "Success"), "You connected successfuly"
        );
    }
}

bool ApplicationEventHandlers::joinCreateDocImpl(const msg::Type type, msg::OneByteInt version, Application& app, const Event& pEvent) {
    auto& params = pEvent.params;
    assert(params.size() > 0);
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
    app.tcpClient.sendMsg(type, version, socket, params[0]);
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