#include "user_history.h"
#include "pos_helpers.h"

UserHistory::UserHistory(std::chrono::milliseconds mergeIntervalMs, const int historyLimit) :
	mergeIntervalMs(mergeIntervalMs),
	historyLimit(historyLimit) {
	undoActions.reserve(historyLimit);
	redoActions.reserve(historyLimit);
}

UserHistory& UserHistory::operator=(const UserHistory& other) {
	undoActions = other.undoActions;
	redoActions = other.redoActions;
	return *this;
}

void UserHistory::push(ActionPtr& action) {
	if (inMergeInterval(action) && undoActions.back()->tryMerge(action)) {
		return;
	}

	redoActions = {};
	if (undoActions.size() >= historyLimit) {
		undoActions.erase(undoActions.begin());
	}

	undoActions.emplace_back(action);
}

std::optional<ActionPtr> UserHistory::undo() {
	if (undoActions.empty()) {
		return {};
	}
	std::optional<ActionPtr> opt{ undoActions.back() };
	redoActions.emplace_back(std::move(undoActions.back()));
	undoActions.pop_back();
	return opt;
}

std::optional<ActionPtr> UserHistory::redo() {
	if (redoActions.empty()) {
		return {};
	}
	std::optional<ActionPtr> opt{ redoActions.back() };
	undoActions.emplace_back(std::move(redoActions.back()));
	redoActions.pop_back();
	return opt;
}

bool UserHistory::inMergeInterval(const ActionPtr& action) const {
	return !undoActions.empty() && action->getTimestamp() < undoActions.back()->getTimestamp() + mergeIntervalMs;
}

void UserHistory::affect(const ActionPtr& newAction) {
	std::vector<std::pair<std::vector<ActionPtr>::const_iterator, ActionPtr>> newActions;
	std::vector<std::vector<ActionPtr>::const_iterator> delActions;
	for (auto actionIt = undoActions.begin(); actionIt != undoActions.end(); actionIt++) {
		auto result = newAction->affect(*actionIt);
		if (actionIt->get()->getTextSize() == COORD{0, 1}) {
			delActions.push_back(actionIt);
		}
		if (result.has_value()) {
			newActions.emplace_back(actionIt, std::move(result.value()));
		}
	}
	for (auto& delAction : delActions) {
		undoActions.erase(delAction);
	}
	for (auto& newAction : newActions) {
		undoActions.insert(newAction.first + 1, newAction.second);
	}
}