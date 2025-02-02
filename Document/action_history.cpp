#include <algorithm>

#include "action_history.h"

ActionHistory::ActionHistory(std::chrono::milliseconds mergeInterval, const int capacity) :
	mergeInterval(mergeInterval),
	capacity(capacity) {
	undoActions.reserve(capacity);
	undoActions.reserve(capacity);
}

void ActionHistory::push(ActionPtr& action) {
	if (tryMerge(action)) {
		return;
	}
	redoActions.clear();
	if (undoActions.size() >= capacity) {
		undoActions.erase(undoActions.begin());
	}
	undoActions.emplace_back(std::move(action));
}

std::optional<ActionHistory::ActionPtr> ActionHistory::undo() {
	if (undoActions.empty()) {
		return {};
	}
	std::optional<ActionPtr> opt{ std::move(undoActions.back()) };
	undoActions.pop_back();
	return opt;
}

std::optional<ActionHistory::ActionPtr> ActionHistory::redo() {
	if (redoActions.empty()) {
		return {};
	}
	std::optional<ActionPtr> opt{ std::move(redoActions.back()) };
	redoActions.pop_back();
	return opt;
}

bool ActionHistory::tryMerge(const ActionPtr& action) const {
	if (undoActions.empty()) {
		return false;
	}
	auto& last = undoActions.back();
	if (action->getTimestamp() >= last->getTimestamp() + mergeInterval) {
		return false;
	}
	return last->tryMerge(action);
}

void ActionHistory::affect(ActionPtr& action) {
	int i = 0;
	while (i < undoActions.size()) {
		int current = i;
		auto result = action->affect(*undoActions[i]);
		if (result.first.has_value()) {
			undoActions.insert(undoActions.cbegin() + current + 1, std::move(result.first.value()));
			i++;
		}
		i++;
	}
}

const std::vector<ActionHistory::ActionPtr>& ActionHistory::getUndoActions() const {
	return undoActions;
}

const std::vector<ActionHistory::ActionPtr>& ActionHistory::getRedoActions() const {
	return redoActions;
}

void ActionHistory::pushToRedo(ActionPtr& action) {
	redoActions.emplace_back(std::move(action));
}

void ActionHistory::pushToUndo(ActionPtr& action) {
	undoActions.emplace_back(std::move(action));
}