#pragma once
#include "window_base.h"
#include "windows_manager.h"


class Application;
class ApplicationEventHandlers {
public:
	ApplicationEventHandlers();
	bool processEvent(Application& app, const Event& pEvent);
private:
	using HandlersMap = std::unordered_map<std::string, void(ApplicationEventHandlers::*)(Application& app, const Event& pEvent)>;

	void eventMainMenuLoginRegisterChosen(Application& app, const Event& pEvent);
	void eventLoginPasswordAccepted(Application& app, const Event& pEvent);
	void eventLogout(Application& app, const Event& pEvent);

	void eventMainMenuDisconnectChosen(Application& app, const Event& pEvent);
	void eventMainMenuCreateChosen(Application& app, const Event& pEvent);
	void eventMainMenuJoinChosen(Application& app, const Event& pEvent);
	void eventMainMenuExitChosen(Application& app, const Event& pEvent);
	void eventMainMenuHelpChosen(Application& app, const Event& pEvent);
	void eventMainMenuShowAcCodeChosen(Application& app, const Event& pEvent);

	void eventCreateDoc(Application& app, const Event& pEvent);
	void eventJoinDoc(Application& app, const Event& pEvent);
	bool joinCreateDocImpl(const msg::Type type, msg::OneByteInt version, Application& app, const Event& pEvent);
	bool waitForResponseAndProccessIt(Application& app, const msg::Type type);

	bool validateTextInputWindow(Application& app, const WindowsIt& window) const;
	bool validateConnection(Application& app);

	HandlersMap handlers;
};