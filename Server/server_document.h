#pragma once
#include "document_base.h"
#include "history_manager.h"

class ServerSiteDocument : public BaseDocument {
public:
	ServerSiteDocument();
	ServerSiteDocument(const std::string& text);
	ServerSiteDocument(const std::string& text, const int cursors, const int myUserIdx);
	ServerSiteDocument(const std::string& text, const int cursors, const int myUserIdx, const history::HistoryManagerOptions& historyManagerOptions);
	ServerSiteDocument(ServerSiteDocument&&) noexcept;
	ServerSiteDocument& operator=(ServerSiteDocument&&) noexcept;
	ServerSiteDocument(const ServerSiteDocument&) = delete;
	ServerSiteDocument& operator=(const ServerSiteDocument&) = delete;

	UndoReturn undo(const int index);
	UndoReturn redo(const int index);

	bool addUser() override;
	bool eraseUser(const int index) override;
private:
	void pushWriteAction(const int index, const COORD& startPos, std::vector<std::string>& text, TextContainer* target) override;
	void pushEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& text, TextContainer* target) override;

	history::HistoryManager historyManager;
};
