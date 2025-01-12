#include "user_action_history.h"
#include "pos_helpers.h"
#include "action.h"

UserActionHistory::UserActionHistory(std::chrono::milliseconds mergeIntervalMs):
	mergeIntervalMs(mergeIntervalMs) {
	undoActions.reserve(historyLimit);
	redoActions.reserve(historyLimit);
}

UserActionHistory& UserActionHistory::operator=(const UserActionHistory& other) {
	undoActions = other.undoActions;
	redoActions = other.redoActions;
	return *this;
}

void UserActionHistory::push(ActionPtr& action) {
	if (inMergeInterval(action) && undoActions.back()->tryMerge(action)) {
		return;
	}

	redoActions = {};
	if (undoActions.size() >= historyLimit) {
		undoActions.erase(undoActions.begin());
	}

	undoActions.emplace_back(action);
}

std::optional<ActionPtr> UserActionHistory::undo() {
	if (undoActions.empty()) {
		return {};
	}
	std::optional<ActionPtr> opt{ undoActions.back() };
	redoActions.emplace_back(std::move(undoActions.back()));
	undoActions.pop_back();
	return opt;
}

std::optional<ActionPtr> UserActionHistory::redo() {
	if (redoActions.empty()) {
		return {};
	}
	std::optional<ActionPtr> opt{ redoActions.back() };
	undoActions.emplace_back(std::move(redoActions.back()));
	redoActions.pop_back();
	return opt;
}

int UserActionHistory::getHistoryLimit() const {
	return historyLimit;
}

const std::vector<ActionPtr>& UserActionHistory::getUndoActions() const {
	return undoActions;
}

const std::vector<ActionPtr>& UserActionHistory::getRedoActions() const {
	return redoActions;
}

bool UserActionHistory::inMergeInterval(const ActionPtr& action) const {
	return !undoActions.empty() && action->getTimestamp() < undoActions.back()->getTimestamp() + mergeIntervalMs;
}

void UserActionHistory::affect(const ActionPtr& newAction, const COORD& posDiff, const bool moveOnly) {
	std::vector<std::pair<std::vector<ActionPtr>::const_iterator, ActionPtr>> newActions;
	std::vector<std::vector<ActionPtr>::const_iterator> delActions;
	for (auto actionIt = undoActions.begin(); actionIt != undoActions.end(); actionIt++) {
		auto result = actionIt->get()->affect(newAction, posDiff, moveOnly);
		if (result.first) {
			delActions.push_back(actionIt);
		}
		if (result.second.has_value()) {
			newActions.emplace_back(actionIt, std::move(result.second.value()));
		}
	}
	for (auto& delAction : delActions) {
		undoActions.erase(delAction);
	}
	for (auto& newAction : newActions) {
		undoActions.insert(newAction.first + 1, newAction.second);
	}
}

