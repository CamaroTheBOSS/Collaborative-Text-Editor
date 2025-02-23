#pragma once
#include "windows_manager.h"
#include "terminal.h"
#include "repository.h"

using Windows = std::vector<std::unique_ptr<BaseWindow>>;
using WindowsRegistry = std::unordered_map<std::string, bool>;
using WindowsIt = Windows::const_iterator;
class Application {
	friend class SyncTester;
public:
	Application(const std::string& ip, const int port);
	bool connect(const std::string& ip, const int port);
	bool disconnect();
	bool isConnected() const;
	KeyPack readChar() const;
	bool processChar(const KeyPack& key);
	bool checkIncomingMessages();
	bool checkBufferWasResized();
	bool waitForDocument(const std::chrono::milliseconds& timeout, const int tries);
	void render();
private:
	bool processEvent(const Event& pEvent);
	void createDocWindow(const TCPClient& client, const std::vector<std::string>& args);
	void createDoc(const TCPClient& client, const std::vector<std::string>& args);
	void loadDocWindow(const TCPClient& client, const std::vector<std::string>& args);
	void loadDoc(const TCPClient& client, const std::vector<std::string>& args);
	void exitApp(const TCPClient& client, const std::vector<std::string>& args);
	void helpWindow(const TCPClient& client, const std::vector<std::string>& args);
	bool loadCreateDoc(const msg::Type type, const TCPClient& client, const std::vector<std::string>& args);

	TCPClient client;
	Terminal terminal;
	client::Repository repo;
	EventHandlersMap<Application> eventHandlers;

	WindowsManager windowsManager;

	std::string srvIp;
	int srvPort;
};
