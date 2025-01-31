#pragma once
#include <optional>
#include <unordered_map>
#include <vector>
#include <chrono>

#include "storage.h"
#include "action.h"

class ActionHistory {
public:
	using ActionPtr = Action::ActionPtr;

	ActionHistory(std::chrono::milliseconds mergeInterval, const int capacity);
	void push(ActionPtr& action);
	std::optional<ActionPtr> redo();
	std::optional<ActionPtr> undo();
	void affect(ActionPtr& action);

	
	const std::vector<ActionPtr>& getUndoActions() const;
	const std::vector<ActionPtr>& getRedoActions() const;
	void pushToRedo(ActionPtr& action);
	void pushToUndo(ActionPtr& action);
private:
	bool tryMerge(const ActionPtr& action) const;

	std::vector<ActionPtr> undoActions;
	std::vector<ActionPtr> redoActions;
	std::chrono::milliseconds mergeInterval;
	int capacity = 2000;
};