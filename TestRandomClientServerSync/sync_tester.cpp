#include "sync_tester.h"
#include "pos_helpers.h"
#include <iostream>
#include <sstream>

SyncTester::SyncTester(const std::string& serverIp, const int serverPort, const int seed) :
	server(serverIp, serverPort),
	client(serverIp, serverPort, seed) {}

void SyncTester::writeToFile(const std::string& filename, const std::string& data) {
	std::ofstream file(filename, std::ios::out);
	if (!file) {
		return;
	}
	file << data;
	file.close();
}

std::string SyncTester::getActionLog(const State& state) const {
	std::stringstream ss;
	for (int i = 0; i < state.actions.size(); i++) {
		ss << i + 1 << ". " + state.actions[i].str() + "\n";
		if (i == 0) {
			continue;
		}
		ss << "Cursor positions:";
		for (int j = 0; j < state.cursorPositions[i].size(); j++) {
			ss << "(" << state.cursorPositions[i-1][j].X << ", " << state.cursorPositions[i-1][j].Y << ") ->";
			ss << "(" << state.cursorPositions[i][j].X << ", " << state.cursorPositions[i][j].Y << ")\n";
		}
	}
	return ss.str();
}

void SyncTester::logDesync(const std::string& errorLog) {
	auto clientActionLog = getActionLog(clientState);
	auto serverActionLog = getActionLog(serverState);
	std::cout << "\n\n\nFound desynchronizations!!!\n";
	std::cout << "Performed actions: ";
	std::cout << clientActionLog;
	std::cout << "Error log:\n" << errorLog;
	writeToFile("clientactions.txt", clientActionLog);
	writeToFile("serveractions.txt", serverActionLog);
	writeToFile("desync.txt", errorLog);
}

std::string SyncTester::compareDocs(const Document& clientDoc, const Document& serverDoc) const {
	std::stringstream ss;
	int nError = 0;
	auto& clientData = clientDoc.get();
	auto& serverData = serverDoc.get();
	auto clientCursors = clientDoc.getCursorPositions();
	auto serverCursors = serverDoc.getCursorPositions();
	for (int i = 0; i < clientCursors.size(); i++) {
		if (clientCursors[i] != serverCursors[i]) {
			nError++;
			ss << nError << ". ClientDoc.cursor" << i << " = " << clientCursors[i].X << "," << clientCursors[i].Y << "\n";
			ss << "   ServerDoc.cursor" << i << " = " << serverCursors[i].X << "," << serverCursors[i].Y << "\n";
		}
	}
	if (clientData.size() != serverData.size()) {
		nError++;
		ss << nError << ". ClientDoc.size() = " << clientData.size() << "\n";
		ss << "   Serverdoc.size() = " << serverData.size() << "\n";
	}
	int minSize = (std::min)(clientData.size(), serverData.size());
	for (int i = 0; i < minSize; i++) {
		if (clientData[i] != serverData[i]) {
			nError++;
			ss << nError << ". ClientDoc:" <<i << " = " << clientData[i] << "\n";
			ss << "   ServerDoc:" << i << " = " << serverData[i] << "\n";
		}
	}
	return ss.str();
}

void SyncTester::randomUndoRedoTest() {
	bool success = true;
	std::string desyncLog;
	constexpr int nRandomActions = 500;
	auto& serverDoc = server.server.repo.doc;
	auto& clientDoc = client.controller.repo.doc;
	serverState.cursorPositions.push_back(serverDoc.getCursorPositions());
	serverState.actions.push_back(DocAction{});
	clientState.cursorPositions.push_back(clientDoc.getCursorPositions());
	clientState.actions.push_back(DocAction{});
	for (int i = 0; i < nRandomActions; i++) {
		
		DocAction key = client.getRandomKey();
		if (key.type == msg::Type::write) {
			key.text = client.getRandomString();
			client.controller.terminal.setClipboardData(key.text);
			key.key.keyCode = CTRL_V;
		}
		client.makeActionSync(key);
		serverState.cursorPositions.push_back(serverDoc.getCursorPositions());
		serverState.actions.push_back(key);
		clientState.cursorPositions.push_back(clientDoc.getCursorPositions());
		clientState.actions.push_back(key);
		desyncLog = compareDocs(clientDoc, serverDoc);
		if (!desyncLog.empty()) {
			desyncLog += "Desync during filling doc on action" + std::to_string(i) + "\n";
			success = false;
			break;
		}
	}
	bool gotMsg = true;
	int msgIndex = 0;
	while (gotMsg && success) {
		DocAction key = client.getUndoAction();
		gotMsg = client.makeActionSync(key);
		serverState.cursorPositions.push_back(serverDoc.getCursorPositions());
		serverState.actions.push_back(key);
		clientState.cursorPositions.push_back(clientDoc.getCursorPositions());
		clientState.actions.push_back(key);
		desyncLog = compareDocs(clientDoc, serverDoc);
		if (!desyncLog.empty()) {
			desyncLog += "Desync during undoing things on action" + std::to_string(msgIndex) + "\n";
			success = false;
			break;
		}
		msgIndex++;
	}
	if (success) {
		std::cout << "SUCCESS!\n";
	}
	else {
		std::cout << "FAIL!\n";
		logDesync(desyncLog);
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