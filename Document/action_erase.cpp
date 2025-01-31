#include "action_erase.h"
#include "pos_helpers.h"
#include "document.h"
#include "action_write.h"
#include "parser.h"
#include <assert.h>

using ActionPtr = EraseAction::ActionPtr;

EraseAction::EraseAction(const COORD& startPos, const COORD& endPos, std::vector<std::string>& text, TextContainer* target, Storage<ActionPtr>* eraseRegistry) :
	Action(ActionType::erase, startPos, text, target, eraseRegistry),
	end(endPos) {}

EraseAction::EraseAction(const COORD& startPos, const COORD& endPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, Storage<ActionPtr>* eraseRegistry) :
	Action(ActionType::erase, startPos, text, target, timestamp, eraseRegistry),
	end(endPos) {}

EraseAction::EraseAction(const COORD& startPos, const COORD& endPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, const bool isChild, Storage<ActionPtr>* eraseRegistry) :
	Action(ActionType::erase, startPos, text, target, timestamp, isChild, eraseRegistry),
	end(endPos) {}

AffectPair EraseAction::affect(Action& other) {
	return other.affectErase(*this);
}

AffectPair EraseAction::affectWrite(Action& other) {
	COORD thisEnd = getEndPos();
	COORD otherStart = other.getStartPos();
	COORD otherEnd = other.getEndPos();
	if ((other.isChild() && otherStart <= thisEnd) || otherStart < thisEnd) {
		move(otherStart, otherEnd - otherStart);
	}
	return {};
}

AffectPair EraseAction::affectErase(Action& other) {
	COORD thisStart = getStartPos();
	COORD thisEnd = getEndPos();
	COORD otherStart = other.getStartPos();
	COORD otherEnd = other.getEndPos();
	if (otherEnd <= thisEnd && otherStart >= thisEnd) {
		end.setPosition(otherEnd);
		if (thisEnd.Y == thisStart.Y) {
			start.setPosition(COORD{ static_cast<SHORT>(otherEnd.X + (thisStart.X - thisEnd.X)), thisStart.Y });
		}
		else {
			start.setPosition(COORD{ thisStart.X, static_cast<SHORT>(thisStart.Y - (thisEnd.Y - otherEnd.Y)) });
		}
	}
	else if (otherEnd <= thisEnd && otherStart <= thisEnd) {
		move(otherStart, otherEnd - otherStart);
	}
	return {};
}

bool EraseAction::tryMerge(const ActionPtr& other) {
	if (other->getType() != ActionType::erase || getEndPos() != other->getStartPos()) {
		return false;
	}
	container = other->container.merge(container);
	end.setPosition(other->getEndPos());
	return true;
}

Action::UndoPair EraseAction::undo() {
	if (target == nullptr) {
		return {};
	}
	COORD startPos = target->validatePos(getEndPos());
	if (startPos == COORD{-1, -1}) {
		COORD startPos = target->validatePos(getEndPos());
	}
	COORD endPos = target->insert(startPos, container.get());
	UndoReturn undoReturn{ ActionType::write, startPos, endPos, container.getText()};
	ActionPtr action = std::move(std::make_unique<WriteAction>(startPos, container, target, getTimestamp(), true, eraseRegistry));
	return std::make_pair<ActionPtr, UndoReturn>(std::move(action), std::move(undoReturn));
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

void EraseAction::triggerRelatedActions() {
	if (eraseRegistry == nullptr) {
		return;
	}
	for (const auto& key : relationshipRegistry) {
		auto it = eraseRegistry->find(key);
		if (it == eraseRegistry->cend()) {
			continue;
		}
		it->second->undo();
		eraseRegistry->erase(it);
	}
}

void EraseAction::move(const COORD& otherStartPos, const COORD& diff) {
	auto thisStartPos = getStartPos();
	auto thisEndPos = getEndPos();
	if (otherStartPos.Y == thisEndPos.Y && otherStartPos.X <= thisEndPos.X) {
		end.setPosition(end.position() + diff);
		if (thisStartPos.Y == thisEndPos.Y) {
			start.setPosition(start.position() + diff);
		}
		else {
			start.setPosition(start.position() + COORD{ 0, diff.Y });
		}
	}
	else if (otherStartPos.Y < thisEndPos.Y) {
		start.setPosition(start.position() + COORD{ 0, diff.Y });
		end.setPosition(end.position() + COORD{ 0, diff.Y });
	}
}