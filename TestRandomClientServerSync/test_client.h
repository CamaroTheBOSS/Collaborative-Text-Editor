#pragma once
#include "controller.h"

#include <thread>
#include <random>
#include <string>
#include <sstream>

struct DocAction {
	KeyPack key{0, 0};
	msg::Type type{msg::Type::error};
	int eraseSize{0};
	std::string text;
	std::string str() const {
		std::stringstream ss;
		ss << "(" << type;
		if (type == msg::Type::erase) {
			ss << " " << eraseSize;
		}
		else if (type == msg::Type::write) {
			ss << " \"" + text + '"';
		}
		ss << ")";
		return ss.str();
	}
};

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
	std::mt19937 randomEngine;
	std::string serverIp;
	int serverPort;
};