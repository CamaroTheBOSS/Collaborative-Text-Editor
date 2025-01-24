#pragma once
#include <optional>
#include <vector>
#include "action.h"

using ActionPtr = std::shared_ptr<Action>;
class UserHistory {
public:
	UserHistory(std::chrono::milliseconds mergeIntervalMs, const int historyLimit);
	UserHistory& operator=(const UserHistory& other);

	void push(ActionPtr& newAction);
	void affect(const ActionPtr& newAction);
	std::optional<ActionPtr> undo();
	std::optional<ActionPtr> redo();
	
private:
	bool inMergeInterval(const ActionPtr& action) const;

	std::vector<ActionPtr> undoActions;
	std::vector<ActionPtr> redoActions;
	std::chrono::milliseconds mergeIntervalMs;
	const int historyLimit = 2000;
};