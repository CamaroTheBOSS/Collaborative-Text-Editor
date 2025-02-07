#pragma once
#include <optional>
#include <vector>
#include <chrono>
#include "action.h"

class ActionHistory {
public:
	using ActionPtr = Action::ActionPtr;

	ActionHistory(std::chrono::milliseconds mergeInterval, const int capacity);
	void push(ActionPtr& action);
	std::optional<ActionPtr> redo();
	std::optional<ActionPtr> undo();
	void affectUndo(ActionPtr& action);
	void affectRedo(ActionPtr& action);

	
	const std::vector<ActionPtr>& getUndoActions() const;
	const std::vector<ActionPtr>& getRedoActions() const;
	void pushToRedo(ActionPtr& action);
	void pushToUndo(ActionPtr& action);
private:
	void _affect(std::vector<ActionPtr>&  actions, ActionPtr& action);
	bool tryMerge(const ActionPtr& action) const;

	std::vector<ActionPtr> undoActions;
	std::vector<ActionPtr> redoActions;
	std::chrono::milliseconds mergeInterval;
	int capacity = 2000;
};