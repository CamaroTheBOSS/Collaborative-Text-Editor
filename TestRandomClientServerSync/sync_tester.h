#pragma once
#include "test_client.h"
#include "test_server.h"
#include "state.h"
#include <fstream>

class SyncTester {
public:
	SyncTester(const std::string& serverIp, const int serverPort, const int seed);
	void start();
private:
	void writeToFile(const std::string& filename, const std::string& data);
	void logDesync(const std::string& errorLog);
	void randomTest();
	void randomUndoRedoTest();
	std::string getActionLog(const State& state) const;
	std::string compareDocs(const Document& clientDoc, const Document& serverDoc) const;
	TestServer server;
	TestClient client;
	std::string filename{"actions.txt"};

	State serverState;
	State clientState;
};