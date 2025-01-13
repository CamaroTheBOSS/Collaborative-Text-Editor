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
	auto type = other->getType();
	auto thisLeft = getLeft();
	auto thisRight = getRight();
	if (type == ActionType::write) {
		auto moveStartPos = other->getStartPos();
		if (thisLeft.Y == moveStartPos.Y && thisLeft.X >= moveStartPos.X) {
			left.setPosition(thisLeft + posDiff);
		}
		else if (thisLeft.Y > moveStartPos.Y) {
			left.setPosition(thisLeft + COORD{ 0, posDiff.Y });
		}
		if (thisRight.Y == moveStartPos.Y && thisRight.X > moveStartPos.X) {
			right.setPosition(thisRight + posDiff);
		}
		else if (thisRight.Y > moveStartPos.Y) {
			right.setPosition(thisRight + COORD{ 0, posDiff.Y });
		}
	}
}

void WriteAction::moveRightCursor(const ActionPtr& other, const COORD& posDiff) {}

Action::ModifyResult WriteAction::modify(const ActionPtr& other) {
	auto type = other->getType();
	if (type == ActionType::erase) {
		return modifyErase(other);
	}
	else if (type == ActionType::write) {
		return modifyWrite(other);
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

	COORD otherLeft = other->getLeft();
	COORD otherRight = other->getRight();
	auto diffToLeft = otherLeft - getLeft();
	int offsetLeft = 0;
	for (int y = 0; y < diffToLeft.Y; y++) {
		offsetLeft = text.find('\n', offsetLeft) + 1;
	}
	offsetLeft += otherLeft.X;
	if (leftBetween && rightBetween) {
		auto newAction = std::make_shared<WriteAction>(*this);
		newAction->setLeft(otherRightPos);
		newAction->text = text.substr(offsetLeft);
		text = text.substr(0, offsetLeft);
		setRight(otherLeftPos);
		return { false, std::optional<ActionPtr>{std::move(newAction)} };
	}
	return { false, std::optional<ActionPtr>{} };
}

Action::ModifyResult WriteAction::modifyErase(const ActionPtr& other) {
	COORD leftPos = getLeft();
	COORD rightPos = getRight();
	COORD otherLeftPos = other->getLeft();
	COORD otherRightPos = other->getRight();
	COORD dL = COORD{ 0, 0 };
	COORD dR = COORD{ 0, 0 };
	if (otherLeftPos.Y < leftPos.Y) {
		COORD posDiff = otherLeftPos - otherRightPos;
		dL.Y += posDiff.Y;
		dR.Y += posDiff.Y;
	}

	COORD base = (std::max)(otherLeftPos, COORD{0, leftPos.Y});
	if (otherRightPos < base) {
		left.setPosition(leftPos + dL);
		right.setPosition(leftPos + dR);
		return { false, std::optional<ActionPtr>{} }; //end
	}
	COORD eraseLen = otherRightPos - base;
	COORD diffLefts = leftPos - base;
	COORD diffStarts = rightPos - base;
	dR = dR + (std::max)((std::min)(eraseLen, diffStarts), COORD{ 0, 0 });
	dL = dL + (std::max)((std::min)(eraseLen, diffLefts), COORD{ 0, 0 });
	COORD newLeft = leftPos - dL;
	COORD newRight = rightPos - dR;
	if (newLeft >= newRight) {
		return { true, std::optional<ActionPtr>{} }; //delete action
	}

	bool leftBetween = posBetween(otherLeftPos, leftPos, rightPos);
	bool rightBetween = posBetween(otherRightPos, leftPos, rightPos);
	int offLeft = 0;
	auto diffToLeft = otherLeftPos - leftPos;
	for (int y = 0; y < diffToLeft.Y; y++) {
		offLeft = text.find('\n', offLeft) + 1;
	}
	offLeft += otherLeftPos.X;
	if (otherLeftPos.Y == leftPos.Y) {
		offLeft -= leftPos.X;
	}

	if (leftBetween && rightBetween) {
		std::string txt = text.substr(0, offLeft);
		text = txt + text.substr(offLeft + other->text.size());
	}
	else if (leftBetween) {
		text = text.substr(0, offLeft);
	}
	else if (rightBetween) {
		int offRight = 0;
		auto diffLeftToRight = otherRightPos - leftPos;
		for (int y = 0; y < diffLeftToRight.Y; y++) {
			offRight = text.find('\n', offRight) + 1;
		}
		offRight += otherRightPos.X;
		if (otherRightPos.Y == leftPos.Y) {
			offRight -= leftPos.X;
		}
		text = text.substr(offRight);
	}
	else {
		leftBetween = posBetween(leftPos, otherLeftPos, otherRightPos);
		rightBetween = posBetween(rightPos, otherLeftPos, otherRightPos);
		bool sameActionBounds = leftPos == otherLeftPos && rightPos == otherRightPos;
		if (leftBetween && rightBetween || sameActionBounds) {
			return { true, std::optional<ActionPtr>{} };
		}
	}
	left.setPosition(newLeft);
	right.setPosition(newRight);
	return { false, std::optional<ActionPtr>{} };
}

