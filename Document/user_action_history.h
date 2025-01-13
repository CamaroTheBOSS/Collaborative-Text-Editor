#pragma once
#include <optional>
#include <vector>
#include <chrono>
#include "action.h"

using ActionPtr = std::shared_ptr<Action>;
class UserActionHistory {
public:
	UserActionHistory(std::chrono::milliseconds mergeIntervalMs);

	UserActionHistory& operator=(const UserActionHistory& other);
	void push(ActionPtr& action);
	std::optional<ActionPtr> undo();
	std::optional<ActionPtr> redo();
	void affect(const ActionPtr& action, const COORD& posDiff);
	int getHistoryLimit() const;
	const std::vector<ActionPtr>& getUndoActions() const;
	const std::vector<ActionPtr>& getRedoActions() const;
private:
	bool inMergeInterval(const ActionPtr& action) const;

	std::vector<ActionPtr> undoActions;
	std::vector<ActionPtr> redoActions;
	std::chrono::milliseconds mergeIntervalMs;
	const int historyLimit = 2000;
};
