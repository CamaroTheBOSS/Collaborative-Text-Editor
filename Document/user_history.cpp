#include "user_history.h"
#include "pos_helpers.h"

UserHistory::UserHistory(std::chrono::milliseconds mergeIntervalMs, const int historyLimit) :
	mergeIntervalMs(mergeIntervalMs),
	historyLimit(historyLimit) {
	undoActions.reserve(historyLimit);
	redoActions.reserve(historyLimit);
}

UserHistory::UserHistory(UserHistory&& other) noexcept :
	mergeIntervalMs(other.mergeIntervalMs),
	historyLimit(other.historyLimit),
	undoActions(std::move(other.undoActions)),
	redoActions(std::move(other.redoActions)) {}

UserHistory& UserHistory::operator=(UserHistory&& other) noexcept {
	undoActions = std::move(other.undoActions);
	redoActions = std::move(other.redoActions);
	return *this;
}

void UserHistory::push(ActionPtr& action) {
	if (inMergeInterval(action) && undoActions.back()->tryMerge(action)) {
		return;
	}

	redoActions.clear();
	if (undoActions.size() >= historyLimit) {
		undoActions.erase(undoActions.begin());
	}

	undoActions.emplace_back(std::move(action));
}

std::optional<ActionPtr> UserHistory::undo() {
	if (undoActions.empty()) {
		return {};
	}
	redoActions.emplace_back(undoActions.back()->convertToOppositeAction());
	std::optional<ActionPtr> opt{ std::move(undoActions.back()) };
	undoActions.pop_back();
	return opt;
}

std::optional<ActionPtr> UserHistory::redo() {
	if (redoActions.empty()) {
		return {};
	}
	undoActions.emplace_back(redoActions.back()->convertToOppositeAction());
	std::optional<ActionPtr> opt{ std::move(redoActions.back()) };
	redoActions.pop_back();
	return opt;
}

const std::vector<ActionPtr>& UserHistory::getUndoActions() const {
	return undoActions;
}

const std::vector<ActionPtr>& UserHistory::getRedoActions() const {
	return redoActions;
}

bool UserHistory::inMergeInterval(const ActionPtr& action) const {
	return !undoActions.empty() && action->getTimestamp() < undoActions.back()->getTimestamp() + mergeIntervalMs;
}

void UserHistory::affect(const ActionPtr& newAction) {
	int i = 0;
	while (i < undoActions.size()) {
		int current = i;
		auto result = newAction->affect(undoActions[i]);
		if (result.has_value()) {
			undoActions.insert(undoActions.cbegin() + current + 1, std::move(result.value()));
			i++;
		}
		if (undoActions[current]->getTextSize() <= COORD{0, 1}) {
			undoActions.erase(undoActions.cbegin() + current);
			i--;
		}
		i++;
	}
}