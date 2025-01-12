#include "action.h"
#include "pos_helpers.h"

Action::Action(ActionType&& type, COORD&& left, COORD&& right, std::string&& txt) :
	type(type),
	left(Cursor(left)),
	right(Cursor(right)),
	text(txt),
	timestamp(std::chrono::system_clock::now()) {}

Action::ModifyResult Action::affect(const ActionPtr& other, const COORD& posDiff, bool moveOnly) {
	moveLeftCursor(other, posDiff);
	moveRightCursor(other, posDiff);
	if (moveOnly) {
		return { false, std::optional<ActionPtr>{} };
	}
	return modify(other);
}

void Action::setLeft(const COORD newPos) {
	left.setPosition(newPos);
}

void Action::setRight(const COORD newPos) {
	right.setPosition(newPos);
}

COORD Action::getLeft() const {
	return left.position();
}

COORD Action::getRight() const {
	return right.position();
}

ActionType Action::getType() const {
	return type;
}

std::string Action::getText() const {
	return text;
}

Timestamp Action::getTimestamp() const {
	return timestamp;
}

std::pair<int, int> Action::getOffsets(const ActionPtr& other) const {
	COORD otherLeft = other->getLeft();
	COORD otherRight = other->getRight();
	auto diffToLeft = diffPos(otherLeft, getLeft());
	auto diffToRight = diffPos(otherLeft, getRight());
	int offsetLeft = 0;
	int offsetRight = 0;
	for (int y = 0; y < diffToLeft.Y; y++) {
		offsetLeft = text.find('\n', offsetLeft) + 1;
	}
	for (int y = 0; y < diffToRight.Y; y++) {
		offsetRight = other->text.find('\n', offsetRight) + 1;
	}
	offsetLeft += otherLeft.X;
	offsetRight += offsetLeft + otherRight.X;
	return { offsetLeft, offsetRight };
}
