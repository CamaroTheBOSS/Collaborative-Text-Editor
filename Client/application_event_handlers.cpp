#include "application_event_handlers.h"
#include "application.h"
#include "validator.h"
#include "window_text_input_obsfucated.h"

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
    double left = getCenteredLeft(screenSize, width);
    app.windowsManager.showWindow<ObsfucatedTextInputWindow>(
        makeGenericBuilder(screenSize, windows::password::name, left, 0.35, width, 1),
        funcSubmitLoginPasswordEvent(pEvent.params[0] == windows::registration::name)
    );
    app.windowsManager.showWindow<TextInputWindow>(
        makeGenericBuilder(screenSize, windows::login::name, left, 0.2, width, 1),
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
    int width = 20;
    auto screenSize = app.terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    for (const auto& winAndName : checkedWindows) {
        if (winAndName.window == allWindows.cend()) {
            app.windowsManager.destroyWindow(winAndName.complementaryName, app.tcpClient);
            app.windowsManager.showWindow<InfoWindow>(
                makeGenericBuilder(screenSize, "Failed", left, 0.4, width, 3), "Cannot find window " + winAndName.window->get()->name()
            );
            return;
        }
        if (!validateTextInputWindow(app, winAndName.window)) {
            return;
        }
    }
    assert(pEvent.params.size() == 1);
    auto type = pEvent.params[0] == windows::registration::name ? msg::Type::registration : msg::Type::login;
    msg::OneByteInt version = 1;
    if (!validateConnection(app)) {
        return;
    }
    app.tcpClient.sendMsg(type, version, checkedWindows[0].window->get()->getDoc().getText(), checkedWindows[1].window->get()->getDoc().getText());
    if (!waitForResponseAndProccessIt(app, type)) {
        return;
    }
    app.windowsManager.destroyWindow(windows::login::name, app.tcpClient);
    app.windowsManager.destroyWindow(windows::password::name, app.tcpClient);
    app.windowsManager.destroyWindow(windows::mainmenu::name, app.tcpClient);
    app.windowsManager.showWindow<MenuWindow>(
        makeMenuWindowBuilder(screenSize, windows::mainmenu::name), app.getMainMenuOptions()
    );
    app.windowsManager.showWindow<InfoWindow>(
        makeGenericBuilder(screenSize, "Success", left, 0.4, width, 1), pEvent.params[0] + " successful"
    );

    std::string authToken = app.repo.getAuthToken();
    if (authToken.empty()) {
        return;
    }
    auto textEditor = dynamic_cast<TextEditorWindow*>(app.windowsManager.getTextEditor().get());
    assert(textEditor != nullptr);
    textEditor->setAuthToken(authToken);
}

void ApplicationEventHandlers::eventMainMenuDisconnectChosen(Application& app, const Event& pEvent) {
    int width = 25;
    auto screenSize = app.terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    app.disconnect();
    app.windowsManager.showWindow<InfoWindow>(
        makeGenericBuilder(screenSize, "Success", left, 0.4, width, 1), "Logged out successfuly"
    );
    app.windowsManager.destroyWindow(windows::mainmenu::name, app.tcpClient);
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
    int width = 15;
    auto screenSize = app.terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    app.windowsManager.showWindow<InfoWindow>(
        makeGenericBuilder(screenSize, "Access code", left, 0.4, width, 1), app.repo.getAcCode()
    );
}

void ApplicationEventHandlers::eventCreateDoc(Application& app, const Event& pEvent) {
    bool success = joinCreateDocImpl(msg::Type::create, 1, app, pEvent);
    if (success) {
        int width = 26;
        auto screenSize = app.terminal.getScreenSize();
        double left = getCenteredLeft(screenSize, width);
        app.windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(screenSize, "Success", left, 0.4, width, 1),
            "Access code for document: " + app.repo.getAcCode()
        );
    }
}

void ApplicationEventHandlers::eventJoinDoc(Application& app, const Event& pEvent) {
    bool success = joinCreateDocImpl(msg::Type::join, 1, app, pEvent);
    if (success) {
        int width = 25;
        auto screenSize = app.terminal.getScreenSize();
        double left = getCenteredLeft(screenSize, width);
        app.windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(screenSize, "Success", left, 0.4, width, 1),
            "You connected successfuly"
        );
    }
}

bool ApplicationEventHandlers::joinCreateDocImpl(const msg::Type type, msg::OneByteInt version, Application& app, const Event& pEvent) {
    auto& params = pEvent.params;
    assert(params.size() > 0);
    int width = 25;
    auto screenSize = app.terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    if (!app.isLogged()) {
        return false;
    }
    if (app.isConnectedToDoc()) {
        app.windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(app.terminal.getScreenSize(), "Failure", left, 0.4, width, 2), "You are already connected to the document. Please disconnect first."
        );
        return false;
    }
    if (!validateConnection(app)) {
        return false;
    }
    unsigned int socket = 0;
    app.tcpClient.sendMsg(type, version, socket, params[0]);
    if (!waitForResponseAndProccessIt(app, type)) {
        app.windowsManager.destroyWindow(pEvent.src, app.tcpClient);
        return false;
    }
    app.windowsManager.destroyLastWindow(app.tcpClient);
    app.windowsManager.destroyWindow(windows::mainmenu::name, app.tcpClient);
    return true;
}

bool ApplicationEventHandlers::waitForResponseAndProccessIt(Application& app, const msg::Type type) {
    bool success = app.waitForResponse(type, std::chrono::milliseconds(500), 4);
    int width = 25;
    auto screenSize = app.terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    if (!success) {
        app.windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(app.terminal.getScreenSize(), "Failure", left, 0.4, width, 2), "No response from the server"
        );
        app.disconnect();
        return false;
    }
    std::string errorMsg = app.repo.getLastError();
    if (!errorMsg.empty()) {
        app.windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(app.terminal.getScreenSize(), "Failure", left, 0.4, width, 2), errorMsg
        );
        app.disconnect();
        return false;
    }
    
    return true;
}

bool ApplicationEventHandlers::validateTextInputWindow(Application& app, const WindowsIt& window) const {
    auto txt = window->get()->getDoc().getText();
    auto msg = Validator::validateString(txt);
    if (!msg.empty()) {
        int width = 25;
        auto screenSize = app.terminal.getScreenSize();
        double left = getCenteredLeft(screenSize, width);
        auto winMsg = window->get()->name() + " doesn't meet required criteria '" + msg + "'";
        app.windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(screenSize, "Failed", left, 0.4, width, 3), winMsg
        );
        return false;
    }
    return true;
}

bool ApplicationEventHandlers::validateConnection(Application& app) {
    int width = 15;
    auto screenSize = app.terminal.getScreenSize();
    double left = getCenteredLeft(screenSize, width);
    if (!app.connect(app.srvIp, app.srvPort)) {
        app.windowsManager.showWindow<InfoWindow>(
            makeGenericBuilder(app.terminal.getScreenSize(), "Failure", left, 0.4, width, 2), "Cannot connect to the server"
        );
        return false;
    }
    return true;
}
