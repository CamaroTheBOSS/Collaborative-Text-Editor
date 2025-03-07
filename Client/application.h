#pragma once
#include "windows_manager.h"
#include "terminal.h"
#include "repository.h"
#include "application_event_handlers.h"
#include "window_menu.h"

using Windows = std::vector<std::unique_ptr<BaseWindow>>;
using WindowsRegistry = std::unordered_map<std::string, bool>;
using WindowsIt = Windows::const_iterator;
class Application {
	friend class SyncTester;
	friend class ApplicationEventHandlers;
public:
	Application(const std::string& ip, const int port);
	bool connect(const std::string& ip, const int port);
	bool disconnect();
	bool isConnected() const;
	bool isConnectedToDoc() const;
	bool isLogged() const;
	KeyPack readChar() const;
	bool processChar(const KeyPack& key);
	bool checkIncomingMessages();
	bool checkBufferWasResized();
	bool waitForResponse(const msg::Type type, const std::chrono::milliseconds& timeout, const int tries);
	void render();
	std::vector<Option> getMainMenuOptions() const;
private:
	TCPClient tcpClient;
	Terminal terminal;
	client::Repository repo;

	ApplicationEventHandlers eventHandler;
	WindowsManager windowsManager;

	std::string srvIp;
	int srvPort;
};
