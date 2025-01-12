#include "action_write.h"
#include "pos_helpers.h"
#include <assert.h>

WriteAction::WriteAction(COORD&& startPos, COORD&& endPos, std::string&& txt):
	Action(std::move(ActionType::write), std::move(startPos), std::move(endPos), std::move(txt)) {}

bool WriteAction::tryMerge(const ActionPtr& other) {
	if (type != other->type || getEndPos() != other->getStartPos()) {
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
		left.setPosition(pos + posDiff);
	}
	else if (pos.Y > moveStartPos.Y) {
		left.setPosition(pos + COORD{ 0, posDiff.Y });
	}
}

void WriteAction::moveRightCursor(const ActionPtr& other, const COORD& posDiff) {
	auto moveStartPos = other->getStartPos();
	auto pos = right.position();
	if (pos.Y == moveStartPos.Y && pos.X > moveStartPos.X) {
		right.setPosition(pos + posDiff);
	}
	else if (pos.Y > moveStartPos.Y) {
		right.setPosition(pos + COORD{ 0, posDiff.Y });
	}
}

Action::ModifyResult WriteAction::modify(const ActionPtr& other) {
	auto type = other->getType();
	if (type == ActionType::erase) {
		return modifyErase(other);
	}
	else if (type == ActionType::write) {
		modifyWrite(other);
	}
	assert(false && "Unrecognized action type. Aborting...");
	return { false, std::optional<ActionPtr>{} };
}

Action::ModifyResult WriteAction::modifyWrite(const ActionPtr& other) {
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
		newAction->text = text.substr(offLeft);
		text = text.substr(0, offLeft);
		setRight(otherLeftPos);
		return { false, std::optional<ActionPtr>{std::move(newAction)} };
	}
	/*else if (leftBetween) {
		setRight(otherLeftPos);
		text = text.substr(0, offLeft);
	}
	else if (rightBetween) {
		setLeft(otherRightPos);
		text = text.substr(offRight);
	}*/
	else {
		//leftBetween = posBetween(leftPos, otherLeftPos, otherRightPos);
		//rightBetween = posBetween(rightPos, otherLeftPos, otherRightPos);
		//bool sameActionBounds = equalPos(leftPos, otherLeftPos) && equalPos(rightPos, otherRightPos);
		//if (leftBetween && rightBetween || sameActionBounds) {
			return { true, std::optional<ActionPtr>{} };
		//}
	}
	return { false, std::optional<ActionPtr>{} };
}

Action::ModifyResult WriteAction::modifyErase(const ActionPtr& other) {
	COORD leftPos = getLeft();
	COORD rightPos = getRight();
	COORD otherLeftPos = other->getLeft();
	COORD otherRightPos = other->getRight();

	auto diffBetweenLefts = otherLeftPos - leftPos;
	int firstOffset = 0;
	for (int y = 0; y < diffBetweenLefts.Y; y++) {
		firstOffset = text.find('\n', firstOffset) + 1;
	}
	auto diffLeftToRight = otherRightPos - leftPos;
	int offset = 0;
	for (int y = 0; y < diffLeftToRight.Y; y++) {
		offset = text.find('\n', offset) + 1;
	}
	offset += diffLeftToRight.X;
	setLeft(leftPos + diffBetweenLefts);
	text = text.substr(offset);

}

