#pragma once
#include <optional>
#include <vector>
#include "action.h"

using ActionPtr = std::unique_ptr<Action>;
class UserHistory {
public:
	UserHistory(std::chrono::milliseconds mergeIntervalMs, const int historyLimit);
	UserHistory(UserHistory& other) = delete;
	UserHistory& operator=(const UserHistory& other) = delete;
	UserHistory(UserHistory&& other) noexcept;
	UserHistory& operator=(UserHistory&& other) noexcept;

	void push(ActionPtr& newAction);
	void pushToRedo(ActionPtr& newAction);
	void pushToUndo(ActionPtr& newAction);
	void affect(const Action& newAction, const bool moveOnly = false);
	std::optional<ActionPtr> undo();
	std::optional<ActionPtr> redo();
	const std::vector<ActionPtr>& getUndoActions() const;
	const std::vector<ActionPtr>& getRedoActions() const;
private:
	void _affect(std::vector<ActionPtr>& actions, const Action& newAction, const bool moveOnly);
	bool inMergeInterval(const ActionPtr& action) const;

	std::vector<ActionPtr> undoActions;
	std::vector<ActionPtr> redoActions;
	std::chrono::milliseconds mergeIntervalMs;
	int historyLimit = 2000;
};