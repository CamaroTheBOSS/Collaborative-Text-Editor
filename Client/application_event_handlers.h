#pragma once
#include "window_base.h"


class Application;
class ApplicationEventHandlers {
public:
	ApplicationEventHandlers();
	bool processEvent(Application& app, const Event& pEvent);
private:
	struct Args {
		Application& app;
		const std::vector<std::string>& args;
	};
	using HandlersMap = std::unordered_map<std::string, void(ApplicationEventHandlers::*)(const Args&)>;


	void eventMainMenuDisconnectChosen(const Args& input);
	void eventMainMenuCreateChosen(const Args& input);
	void eventMainMenuJoinChosen(const Args& input);
	void eventMainMenuExitChosen(const Args& input);
	void eventMainMenuHelpChosen(const Args& input);
	void eventMainMenuShowAcCodeChosen(const Args& input);

	void eventCreateDoc(const Args& input);
	void eventJoinDoc(const Args& input);
	bool joinCreateDocImpl(const msg::Type type, msg::OneByteInt version, const Args& input);

	HandlersMap handlers;
};