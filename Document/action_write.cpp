#include "action_write.h"
#include "pos_helpers.h"

WriteAction::WriteAction(COORD&& startPos, COORD&& endPos, std::string&& txt):
	Action(std::move(ActionType::write), std::move(startPos), std::move(endPos), std::move(txt)) {}

bool WriteAction::tryMerge(const ActionPtr& other) {
	if (type != other->type || !equalPos(getEndPos(), other->getStartPos())) {
		return false;
	}
	right.setPosition(other->getEndPos());
	text += other->text;
	return true;
}

COORD WriteAction::getStartPos() const {
	return left.position();
}

COORD WriteAction::getEndPos() const {
	return right.position();
}

void WriteAction::moveLeftCursor(const ActionPtr& other, const COORD& posDiff) {
	auto moveStartPos = other->getStartPos();
	auto pos = left.position();
	if (pos.Y == moveStartPos.Y && pos.X >= moveStartPos.X) {
		left.setPosition(sumPos(pos, posDiff));
	}
	else if (pos.Y > moveStartPos.Y) {
		left.setPosition(sumPos(pos, COORD{ 0, posDiff.Y }));
	}
}

void WriteAction::moveRightCursor(const ActionPtr& other, const COORD& posDiff) {
	auto moveStartPos = other->getStartPos();
	auto pos = right.position();
	if (pos.Y == moveStartPos.Y && pos.X > moveStartPos.X) {
		right.setPosition(sumPos(pos, posDiff));
	}
	else if (pos.Y > moveStartPos.Y) {
		right.setPosition(sumPos(pos, COORD{ 0, posDiff.Y }));
	}
}

void WriteAction::move(const ActionPtr& other, Cursor& cursor, const COORD& posDiff) {
	
}

Action::ModifyResult WriteAction::modify(const ActionPtr& other) {
	COORD leftPos = getLeft();
	COORD rightPos = getRight();
	COORD otherLeftPos = other->getLeft();
	COORD otherRightPos = other->getRight();
	bool leftBetween = posBetween(otherLeftPos, leftPos, rightPos);
	bool rightBetween = posBetween(otherRightPos, leftPos, rightPos);
	auto [offLeft, offRight] = getOffsets(other);
	if (leftBetween && rightBetween) {
		auto newAction = std::make_shared<WriteAction>(*this);
		newAction->setLeft(otherRightPos);
		newAction->text = text.substr(other->getType() == ActionType::erase ? offRight : offLeft);
		text = text.substr(0, offLeft);
		setRight(otherLeftPos);
		return { false, std::optional<ActionPtr>{std::move(newAction)}};
	}
	else if (leftBetween) {
		setRight(otherLeftPos);
		text = text.substr(0, offLeft);
	}
	else if (rightBetween) {
		setLeft(otherRightPos);
		text = text.substr(offRight);
	}
	else {
		leftBetween = posBetween(leftPos, otherLeftPos, otherRightPos);
		rightBetween = posBetween(rightPos, otherLeftPos, otherRightPos);
		bool sameActionBounds = equalPos(leftPos, otherLeftPos) && equalPos(rightPos, otherRightPos);
		if (leftBetween && rightBetween || sameActionBounds) {
			return { true, std::optional<ActionPtr>{} };
		}
	}
	return { false, std::optional<ActionPtr>{} };
}
