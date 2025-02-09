#include "server_document.h"
#include "pos_helpers.h"

ServerSiteDocument::ServerSiteDocument() :
	BaseDocument() {
	addUser();
}

ServerSiteDocument::ServerSiteDocument(const std::string& text) :
	BaseDocument(text) {
	addUser();
}

ServerSiteDocument::ServerSiteDocument(const std::string& text, const int nCursors, const int myUserIdx) :
	BaseDocument(text, nCursors, myUserIdx) {
	for (int i = 0; i < nCursors; i++) {
		addUser();
	}
}

ServerSiteDocument::ServerSiteDocument(const std::string& text, const int nCursors, const int myUserIdx, const history::HistoryManagerOptions& historyManagerOptions) :
	BaseDocument(text, nCursors, myUserIdx),
	historyManager(historyManagerOptions) {
	for (int i = 0; i < nCursors; i++) {
		addUser();
	}
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

void ServerSiteDocument::afterWriteAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& writtenText) {
	historyManager.pushWriteAction(index, startPos, writtenText, &container);
}

void ServerSiteDocument::afterEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& erasedText) {
	historyManager.pushEraseAction(index, startPos, endPos, erasedText, &container);
}

