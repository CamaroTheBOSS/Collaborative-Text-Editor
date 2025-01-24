#include "action_erase.h"
#include "pos_helpers.h"

using ActionPtr = EraseAction::ActionPtr;

EraseAction::EraseAction(COORD& startPos, COORD& endPos, std::vector<std::string>& text) :
	Action(startPos, text),
	end(endPos) {}

EraseAction::EraseAction(COORD& startPos, COORD& endPos, std::vector<std::string>& text, Timestamp timestamp) :
	Action(startPos, text, timestamp),
	end(endPos) {}

std::optional<ActionPtr> EraseAction::affect(const ActionPtr& other) {
	return other->affectErase(*this);
}

std::optional<ActionPtr> EraseAction::affectWrite(const Action& other) {
	COORD thisStart = getStartPos();
	COORD otherStart = other.getStartPos();
	COORD otherEnd = other.getEndPos();
	if (otherStart <= thisStart) {
		move(otherStart, otherEnd - otherStart);
	}
	return {};
}

std::optional<ActionPtr> EraseAction::affectErase(const Action& other) {
	COORD thisStart = getStartPos();
	COORD thisEnd = getEndPos();
	COORD otherStart = other.getStartPos();
	COORD otherEnd = other.getEndPos();
	if (otherStart <= thisStart) {
		move(otherStart, otherEnd - otherStart);
	}
	else if (otherEnd < thisStart) {
		start.setPosition(otherEnd);
		end.setPosition(otherEnd);
	}
	return {};
}

bool EraseAction::tryMerge(const ActionPtr& other) {
	if (dynamic_cast<EraseAction*>(other.get()) == nullptr || getEndPos() != other->getStartPos() || moved) {
		return false;
	}
	text = std::move(mergeText(other->text, text));
	end = other->getEndPos();
	return true;
}

COORD EraseAction::getLeftPos() const {
	return end.position();
}

COORD EraseAction::getRightPos() const {
	return getStartPos();
}

COORD EraseAction::getEndPos() const {
	return getLeftPos();
}

void EraseAction::move(const COORD& otherStartPos, const COORD& diff) {
	auto thisPos = getStartPos();
	if (otherStartPos.Y == thisPos.Y && otherStartPos.X <= thisPos.X) {
		start.setPosition(start.position() + diff);
		end.setPosition(end.position() + COORD{ 0, diff.Y });
		moved = true;
	}
	else if (otherStartPos.Y < thisPos.Y) {
		start.setPosition(start.position() + COORD{ 0, diff.Y });
		end.setPosition(end.position() + COORD{ 0, diff.Y });
		moved = true;
	}
}