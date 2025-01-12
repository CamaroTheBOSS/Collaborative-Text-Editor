#include "action_noop.h"

NoopAction::NoopAction() :
	Action(ActionType::noop, COORD{ 0, 0 }, COORD{ 0, 0 }, "") {}

bool NoopAction::tryMerge(const ActionPtr& other) { return true; }

COORD NoopAction::getStartPos() const {
	return COORD{0, 0};
}

COORD NoopAction::getEndPos() const {
	return COORD{ 0, 0 };
}

void NoopAction::moveLeftCursor(const ActionPtr& other, const COORD& posDiff) {}

void NoopAction::moveRightCursor(const ActionPtr& other, const COORD& posDiff) {}

Action::ModifyResult NoopAction::modify(const ActionPtr& other) {
	return { false, std::optional<ActionPtr>{} };
}
