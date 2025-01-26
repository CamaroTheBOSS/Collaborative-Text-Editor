#include "server.h"

#include <string>
#include <thread>

class TestServer {
public:
	TestServer(const std::string& ip, const int port);
	void start();
	void stop();
private:
	friend class SyncTester;
	void run();
	std::thread serverThread;
	Server server;
};