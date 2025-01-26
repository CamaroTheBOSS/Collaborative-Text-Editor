#pragma once
#include "test_client.h"
#include "test_server.h"
#include <fstream>

class SyncTester {
public:
	SyncTester(const std::string& serverIp, const int serverPort, const int seed);
	void start();
private:
	void writeActionsToFile(const std::vector<DocAction>& actions);
	void logDesync(const std::string& errorLog, const std::vector<DocAction>& actions) const;
	void randomTest();
	void randomUndoRedoTest();
	std::string getActionLog(const std::vector<DocAction>& actions) const;
	std::string compareDocs(const Document& clientDoc, const Document& serverDoc) const;
	TestServer server;
	TestClient client;
	std::string filename{"actions.txt"};
};