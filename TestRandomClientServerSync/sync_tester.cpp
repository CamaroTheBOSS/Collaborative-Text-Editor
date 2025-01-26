#include "sync_tester.h"
#include <iostream>

SyncTester::SyncTester(const std::string& serverIp, const int serverPort, const int seed) :
	server(serverIp, serverPort),
	client(serverIp, serverPort, seed) {}

void SyncTester::writeActionsToFile(const std::vector<DocAction>& actions) {
	std::ofstream file(filename, std::ios::out);
	if (!file) {
		return;
	}
	file << getActionLog(actions);
	file.close();
}

std::string SyncTester::getActionLog(const std::vector<DocAction>& actions) const {
	std::string actionLog;
	for (const auto& action : actions) {
		actionLog += action.str() + "\n";
	}
	return actionLog;
}

void SyncTester::logDesync(const std::string& errorLog, const std::vector<DocAction>& actions) const {
	std::cout << "\n\n\nFound desynchronizations!!!\n";
	std::cout << "Performed actions: ";
	std::cout << getActionLog(actions);
	std::cout << "Error log:\n" << errorLog;
}

std::string SyncTester::compareDocs(const Document& clientDoc, const Document& serverDoc) const {
	std::string ret;
	int nError = 0;
	if (clientDoc.data.size() != serverDoc.data.size()) {
		nError++;
		ret += std::to_string(nError) + ". ClientDoc.size() = " + std::to_string(clientDoc.data.size()) + "\n";
		ret += "   Serverdoc.size() = " + std::to_string(serverDoc.data.size());
	}
	int minSize = (std::min)(clientDoc.data.size(), serverDoc.data.size());
	for (int i = 0; i < minSize; i++) {
		if (clientDoc.data[i] != serverDoc.data[i]) {
			nError++;
			ret += std::to_string(nError) + ". ClientDoc:" + std::to_string(i) + " = " + clientDoc.data[i] + "\n";
			ret += "   Serverdoc:" + std::to_string(i) + " = " + serverDoc.data[i] + "\n";
		}
	}
	return ret;
}

void SyncTester::randomTest() {
	constexpr int nRandomActions = 10000;
	auto& serverDoc = server.server.repo.doc;
	auto& clientDoc = client.controller.repo.doc;
	std::vector<DocAction> performedActions;
	for (int i = 0; i < nRandomActions; i++) {
		DocAction key = client.getRandomKey();
		client.makeActionSync(key);
		performedActions.emplace_back(std::move(key));
		auto ret = compareDocs(clientDoc, serverDoc);
		if (!ret.empty()) {
			logDesync(ret, performedActions);
			break;
		}
	}
}

void SyncTester::randomUndoRedoTest() {
	constexpr int nRandomActions = 500;
	auto& serverDoc = server.server.repo.doc;
	auto& clientDoc = client.controller.repo.doc;
	std::vector<DocAction> performedActions;
	for (int i = 0; i < nRandomActions; i++) {
		DocAction key = client.getRandomKey();
		if (key.type == msg::Type::write) {
			key.text = client.getRandomString();
			client.controller.terminal.setClipboardData(key.text);
			key.key.keyCode = CTRL_V;
		}
		client.makeActionSync(key);
		performedActions.emplace_back(std::move(key));
		auto ret = compareDocs(clientDoc, serverDoc);
		if (!ret.empty()) {
			logDesync(ret, performedActions);
			break;
		}
	}
	writeActionsToFile(performedActions);
	bool gotMsg = true;
	while (gotMsg) {
		DocAction key = client.getUndoAction();
		gotMsg = client.makeActionSync(key);
		performedActions.emplace_back(std::move(key));
		auto ret = compareDocs(clientDoc, serverDoc);
		if (!ret.empty()) {
			logDesync(ret, performedActions);
			break;
		}
	}
	return;
}


void SyncTester::start() {
	server.start();
	client.start();
	randomUndoRedoTest();
	std::this_thread::sleep_for(std::chrono::milliseconds{200});
	client.stop();
	server.stop();
}