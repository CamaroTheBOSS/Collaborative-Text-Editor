#pragma once
#include "document_base.h"
#include "history_manager.h"

class ServerSiteDocument : public BaseDocument {
public:
	ServerSiteDocument();
	ServerSiteDocument(const std::string& text);
	ServerSiteDocument(const std::string& text, const int cursors, const int myUserIdx);
	ServerSiteDocument(const std::string& text, const int cursors, const int myUserIdx, const history::HistoryManagerOptions& historyManagerOptions);

	UndoReturn undo(const int index);
	UndoReturn redo(const int index);

	bool addUser() override;
	bool eraseUser(const int index) override;
private:
	void afterWriteAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& writtenText) override;
	void afterEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& erasedText) override;

	history::HistoryManager historyManager;
};
