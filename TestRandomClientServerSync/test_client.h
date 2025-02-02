#pragma once
#include "controller.h"
#include "state.h"

#include <thread>
#include <string>
#include <sstream>

class TestClient {
	friend class SyncTester;
public:
	TestClient(const std::string& serverIp, const int serverPort, const int seed);
	void start();
	void stop();
	bool makeActionSync(const DocAction& key);
	DocAction getRandomKey();
	DocAction getUndoAction() const;
	std::string getRandomString();
private:
	Controller controller;
	std::string serverIp;
	int serverPort;
};