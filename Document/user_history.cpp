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
	std::optional<ActionPtr> opt{ std::move(undoActions.back()) };
	undoActions.pop_back();
	return opt;
}

std::optional<ActionPtr> UserHistory::redo() {
	if (redoActions.empty()) {
		return {};
	}
	std::optional<ActionPtr> opt{ std::move(redoActions.back()) };
	redoActions.pop_back();
	return opt;
}

void UserHistory::pushToRedo(ActionPtr& newAction) {
	redoActions.emplace_back(std::move(newAction));
}
void UserHistory::pushToUndo(ActionPtr& newAction) {
	undoActions.emplace_back(std::move(newAction));
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

void UserHistory::affect(Action& newAction, const bool moveOnly, const bool fromUndo) {
	_affect(undoActions, newAction, moveOnly, fromUndo);
	//_affect(redoActions, newAction, moveOnly);
}

void UserHistory::_affect(std::vector<ActionPtr>& actions, Action& newAction, const bool moveOnly, const bool fromUndo) {
	int i = 0;
	while (i < actions.size()) {
		int current = i;
		auto result = newAction.affect(*actions[i], moveOnly, fromUndo);
		if (result.has_value()) {
			actions.insert(actions.cbegin() + current + 1, std::move(result.value()));
			i++;
		}
		if (actions[current]->getTextSize() <= COORD{0, 1}) {
			actions.erase(actions.cbegin() + current);
			i--;
		}
		i++;
	}
}