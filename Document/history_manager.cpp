#include "history_manager.h"
#include "action_write.h"
#include "action_erase.h"

#include <memory>

namespace history {
	HistoryManager::HistoryManager(const HistoryManagerOptions& options) :
		mergeInterval(options.mergeInterval),
		capacity(options.capacity) {}

	HistoryManager::HistoryManager(HistoryManager&& other) noexcept :
		histories(std::move(other.histories)),
		eraseRegistry(std::move(other.eraseRegistry)) {}

	HistoryManager& HistoryManager::operator=(HistoryManager&& other) noexcept {
		histories = std::move(other.histories);
		eraseRegistry = std::move(other.eraseRegistry);
		return *this;
	}

	HistoryManager::~HistoryManager() {
		histories.clear();
	}

	int HistoryManager::addHistory() {
		histories.emplace_back(ActionHistory(mergeInterval, capacity));
		return histories.size() - 1;
	}

	bool HistoryManager::removeHistory(const int index) {
		histories.erase(histories.cbegin() + index);
		return true;
	}

	void HistoryManager::push(const int index, ActionPtr& action) {
		if (index < 0 || index >= histories.size()) {
			return;
		}
		affect(index, action);
		histories[index].push(action);
	}

	void HistoryManager::affect(const int index, ActionPtr& action) {
		for (int i = 0; i < histories.size(); i++) {
			histories[i].affectUndo(action);
			if (i != index) {
				histories[i].affectRedo(action);
			}
		}
	}

	UndoReturn HistoryManager::undo(const int index) {
		if (index < 0 || index >= histories.size()) {
			return  { ActionType::noop };
		}
		auto actionOpt = histories[index].undo();
		if (!actionOpt.has_value()) {
			return { ActionType::noop };
		}
		auto [actionDone, undoRet] = actionOpt.value()->undo();
		affect(index, actionDone);
		actionOpt.value()->triggerRelatedActions();
		histories[index].pushToRedo(actionDone);
		return undoRet;
	}

	UndoReturn HistoryManager::redo(const int index) {
		if (index < 0 || index >= histories.size()) {
			return  { ActionType::noop };
		}
		auto actionOpt = histories[index].redo();
		if (!actionOpt.has_value()) {
			return { ActionType::noop };
		}
		auto [actionDone, undoRet] = actionOpt.value()->undo();
		affect(index, actionDone);
		actionOpt.value()->triggerRelatedActions();
		histories[index].pushToUndo(actionDone);
		return undoRet;
	}

	void HistoryManager::pushWriteAction(const int index, const COORD& startPos, std::vector<std::string>& text, TextContainer* target) {
		ActionPtr action = std::make_unique<WriteAction>(startPos, text, target, &eraseRegistry);
		push(index, action);
	}

	void HistoryManager::pushEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& text, TextContainer* target) {
		ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, text, target, &eraseRegistry);
		push(index, action);
	}
}
