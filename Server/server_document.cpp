#include "server_document.h"
#include "pos_helpers.h"

ServerSiteDocument::ServerSiteDocument() :
	BaseDocument() {}

ServerSiteDocument::ServerSiteDocument(const std::string& text) :
	BaseDocument(text) {}

ServerSiteDocument::ServerSiteDocument(const std::string& text, const int nCursors, const int myUserIdx) :
	BaseDocument(text, nCursors, myUserIdx) {}

ServerSiteDocument::ServerSiteDocument(const std::string& text, const int nCursors, const int myUserIdx, const history::HistoryManagerOptions& historyManagerOptions) :
	BaseDocument(text, nCursors, myUserIdx),
	historyManager(historyManagerOptions) {}

ServerSiteDocument::ServerSiteDocument(ServerSiteDocument&& other) noexcept :
	BaseDocument(std::move(other)),
	historyManager(std::move(other.historyManager)) {}

ServerSiteDocument& ServerSiteDocument::operator=(ServerSiteDocument&& other) noexcept {
	container = std::move(other.container);
	users = std::move(other.users);
	filename = std::move(other.filename);
	myUserIdx = other.myUserIdx;
	historyManager = std::move(other.historyManager);
	return *this;
}

bool ServerSiteDocument::addUser() {
	users.emplace_back(User());
	historyManager.addHistory();
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

void ServerSiteDocument::pushWriteAction(const int index, const COORD& startPos, std::vector<std::string>& text, TextContainer* target) {
	historyManager.pushWriteAction(index, startPos, text, target);
}

void ServerSiteDocument::pushEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& text, TextContainer* target) {
	historyManager.pushEraseAction(index, startPos, endPos, text, target);
}

