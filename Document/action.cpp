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