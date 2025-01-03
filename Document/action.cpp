#include "action.h"
#include "pos_helpers.h"

Action::Action(ActionType&& type, COORD&& left, COORD&& right, std::string&& txt) :
	type(type),
	left(Cursor(left)),
	right(Cursor(right)),
	text(txt),
	timestamp(std::chrono::system_clock::now()) {}

Action::ModifyResult Action::affect(const ActionPtr& other, const COORD& posDiff) {
	moveLeftCursor(other, posDiff);
	moveRightCursor(other, posDiff);
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
	auto diffLeft = diffPos(otherLeft, getLeft());
	auto diffRight = diffPos(otherRight, getRight());
	int offsetLeft = 0;
	int offsetRight = 0;
	for (int y = 0; y < diffLeft.Y; y++) {
		offsetLeft = other->text.find(offsetLeft, '\n') + 1;
		offsetRight = offsetLeft;
	}
	for (int y = 0; y < diffRight.Y; y++) {
		offsetRight = other->text.find(offsetRight, '\n') + 1;
	}
	offsetLeft += otherLeft.X;
	offsetRight += otherRight.X;
	return { offsetLeft, offsetRight };
}
