#pragma once
#include "document_base.h"
#include "history_manager.h"
#include <Winsock2.h>

class ServerSiteDocument : public BaseDocument {
public:
	ServerSiteDocument();
	ServerSiteDocument(const std::string& text);
	ServerSiteDocument(const std::string& text, const int cursors, const int myUserIdx, const std::string& docName = "filename.txt");
	ServerSiteDocument(const std::string& text, const int cursors, const int myUserIdx, const history::HistoryManagerOptions& historyManagerOptions);

	UndoReturn undo(const int index);
	UndoReturn redo(const int index);

	bool addUser() override;
	bool addClient(SOCKET client);
	bool eraseUser(const int index) override;
	bool eraseClient(SOCKET client);
	int findUser(SOCKET client) const;
	std::vector<SOCKET>& getConnectedClients();
private:
	void afterWriteAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& writtenText) override;
	void afterEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& erasedText) override;

	history::HistoryManager historyManager;
	std::vector<SOCKET> connectedClients;
};
