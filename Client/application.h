#pragma once
#include "repository.h"
#include "tcp_client.h"

#include <chrono>
#include <unordered_map>
#include <memory>

import window.base;
import window.manager;
import window.menu;
import messages;
import terminal;

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
	bool processEvent(const Event& event);
	bool validateConnection();
	bool validateTextInputWindow(const WindowsIt& window);
	bool waitForResponseAndProccessIt(const msg::Type type);
	bool joinCreateDocImpl(const msg::Type type, msg::OneByteInt version, const Event& pEvent);
	void eventJoinDoc(const Event& pEvent);
	void eventCreateDoc(const Event& pEvent);
	void eventLoadItemDeleted(const Event& pEvent);
	void eventLoadItemAccepted(const Event& pEvent);
	void eventLoadItemClicked(const Event& pEvent);
	void eventMainMenuShowAcCodeChosen(const Event& pEvent);
	void eventMainMenuHelpChosen(const Event& pEvent);
	void eventMainMenuExitChosen(const Event& pEvent);
	void eventMainMenuLoadChosen(const Event& pEvent);
	void eventMainMenuJoinChosen(const Event& pEvent);
	void eventMainMenuCreateChosen(const Event& pEvent);
	void eventMainMenuDisconnectChosen(const Event& pEvent);
	void eventLoginPasswordAccepted(const Event& pEvent);
	void eventMainMenuLoginRegisterChosen(const Event& pEvent);

	TCPClient tcpClient;
	Terminal terminal;
	client::Repository repo;

	WindowsManager windowsManager;
	std::unordered_map<std::string, void(*)(const Event&)> eventHandlers;
	std::string srvIp;
	int srvPort;
};
