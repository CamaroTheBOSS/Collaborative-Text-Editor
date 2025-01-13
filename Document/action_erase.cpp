#include "action_erase.h"
#include "pos_helpers.h"

EraseAction::EraseAction(COORD&& startPos, COORD&& endPos, std::string&& txt) :
	Action(std::move(ActionType::erase), std::move(endPos), std::move(startPos), std::move(txt)) {}

bool EraseAction::tryMerge(const ActionPtr& other) {
	if (type != other->type || getEndPos() != other->getStartPos()) {
		return false;
	}
	left.setPosition(other->getEndPos());
	text = other->text + text;
	return true;
}

COORD EraseAction::getStartPos() const {
	return right.position();
}

COORD EraseAction::getEndPos() const {
	return left.position();
}

void EraseAction::moveLeftCursor(const ActionPtr& other, const COORD& posDiff) {
	auto moveStartPos = other->getStartPos();
	auto pos = left.position();
	if (pos.Y == moveStartPos.Y && pos.X >= moveStartPos.X) {
		left.setPosition(pos + posDiff);
		right.setPosition(pos + posDiff);
	}
	else if (pos.Y > moveStartPos.Y) {
		left.setPosition(pos + COORD{ 0, posDiff.Y });
		right.setPosition(pos + COORD{ 0, posDiff.Y });
	}
}

void EraseAction::moveRightCursor(const ActionPtr& other, const COORD& posDiff) {}

Action::ModifyResult EraseAction::modify(const ActionPtr& other) {
	return { false, std::optional<ActionPtr>{} };
}
