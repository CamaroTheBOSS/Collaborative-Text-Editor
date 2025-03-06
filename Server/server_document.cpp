#include "server_document.h"
#include "pos_helpers.h"

ServerSiteDocument::ServerSiteDocument() :
	BaseDocument(),
	id("") {
	addUser();
}

ServerSiteDocument::ServerSiteDocument(const std::string& text) :
	BaseDocument(text),
	id("") {
	addUser();
}

ServerSiteDocument::ServerSiteDocument(const std::string& text, const int nCursors, const int myUserIdx, const std::string& id, const std::string& docName) :
	BaseDocument(text, nCursors, myUserIdx),
	id(id) {
	filename = docName;
	for (int i = 0; i < nCursors; i++) {
		addUser();
	}
}

ServerSiteDocument::ServerSiteDocument(const std::string& text, const int nCursors, const int myUserIdx, const history::HistoryManagerOptions& historyManagerOptions) :
	BaseDocument(text, nCursors, myUserIdx),
	historyManager(historyManagerOptions),
	id("") {
	for (int i = 0; i < nCursors; i++) {
		addUser();
	}
}

int ServerSiteDocument::findUser(SOCKET client) const {
	int userIdx = -1;
	for (int i = 0; i < connectedClients.size(); i++) {
		if (connectedClients[i] == client) {
			userIdx = i;
			break;
		}
	}
	return userIdx;
}

std::vector<SOCKET>& ServerSiteDocument::getConnectedClients() {
	return connectedClients;
}

bool ServerSiteDocument::addUser() {
	users.emplace_back(User());
	historyManager.addHistory();
	return true;
}

bool ServerSiteDocument::addClient(SOCKET client) {
	connectedClients.push_back(client);
	return true;
}

bool ServerSiteDocument::eraseUser(const int index) {
	if (!validateUserIdx(index)) {
		return false;
	}
	historyManager.removeHistory(index);
	users.erase(users.cbegin() + index);
	if (myUserIdx > index) {
		myUserIdx--;
	}
	return true;
}

bool ServerSiteDocument::eraseClient(SOCKET client) {
	int index = findUser(client);
	if (index < 0) {
		return false;
	}
	connectedClients.erase(connectedClients.cbegin() + index);
	return true;
}

UndoReturn ServerSiteDocument::undo(const int index) {
	if (!validateUserIdx(index)) {
		return { ActionType::noop };
	}
	auto ret = historyManager.undo(index);
	if (ret.type == ActionType::noop) {
		return ret;
	}
	users[index].cursor.setPosition(ret.startPos);
	COORD diff = ret.endPos - ret.startPos;
	moveAffectedCursors(users[index], diff);
	return ret;
}

UndoReturn ServerSiteDocument::redo(const int index) {
	if (!validateUserIdx(index)) {
		return { ActionType::noop };
	}
	auto ret = historyManager.redo(index);
	users[index].cursor.setPosition(ret.startPos);
	COORD diff = ret.endPos - ret.startPos;
	moveAffectedCursors(users[index], diff);
	return ret;
}

void ServerSiteDocument::afterWriteAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& writtenText) {
	historyManager.pushWriteAction(index, startPos, writtenText, &container);
}

void ServerSiteDocument::afterEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& erasedText) {
	historyManager.pushEraseAction(index, startPos, endPos, erasedText, &container);
}

ServerSiteDocument::Timestamp ServerSiteDocument::getLastSaveTimestamp() const {
	return lastSaveTimestamp;
}

void ServerSiteDocument::setNowAsLastSaveTimestamp() {
	lastSaveTimestamp = std::chrono::system_clock::now();
}

std::string ServerSiteDocument::getId() const {
	return id;
}
