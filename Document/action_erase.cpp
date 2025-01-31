#include "action_erase.h"
#include "pos_helpers.h"
#include "document.h"
#include "action_write.h"
#include "parser.h"

using ActionPtr = EraseAction::ActionPtr;

EraseAction::EraseAction(COORD& startPos, COORD& endPos, std::vector<std::string>& text) :
	Action(ActionType::erase, startPos, text),
	end(endPos) {}

EraseAction::EraseAction(COORD& startPos, COORD& endPos, std::vector<std::string>& text, Timestamp timestamp) :
	Action(ActionType::erase, startPos, text, timestamp),
	end(endPos) {}

ActionPtr EraseAction::convertToOppositeAction() const {
	auto startPos = getEndPos();
	auto textCopy = text;
	return std::make_unique<WriteAction>(startPos, textCopy);
}

void EraseAction::addUndoHook(ActionSptr& action, const int token) {
	undoHooks.emplace_back(std::make_pair(action, token));
}

std::optional<ActionPtr> EraseAction::affect(Action& other, const bool moveOnly, const bool fromUndo) {
	return other.affectErase(*this, moveOnly, fromUndo);
}

std::optional<ActionPtr> EraseAction::affectWrite(Action& other, const bool moveOnly, const bool fromUndo) {
	COORD thisEnd = getEndPos();
	COORD otherStart = other.getStartPos();
	COORD otherEnd = other.getEndPos();
	if ((fromUndo && otherStart <= thisEnd) || otherStart < thisEnd) {
		move(otherStart, otherEnd - otherStart);
	}
	return {};
}

std::optional<ActionPtr> EraseAction::affectErase(Action& other, const bool moveOnly, const bool fromUndo) {
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
	if (dynamic_cast<EraseAction*>(other.get()) == nullptr || getEndPos() != other->getStartPos() || moved) {
		return false;
	}
	text = std::move(mergeText(other->text, text));
	end = other->getEndPos();
	return true;
}

Action::UndoPair EraseAction::undo(const int userIdx, Document& doc) const {
	doc.setCursorPos(userIdx, (std::min)((std::max)(COORD{ 0, 0 }, getEndPos()), doc.getEndPos()));
	COORD startPos = doc.getCursorPos(userIdx);
	COORD endPos = doc.insertText(startPos, text);
	COORD diffPos = endPos - startPos;
	UndoReturn undoReturn{ ActionType::write, startPos, endPos, Parser::parseVectorToText(text) };
	auto textCopy = text;
	ActionPtr action = std::move(std::make_unique<WriteAction>(startPos, textCopy));
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
		moved = true;
	}
	else if (otherStartPos.Y < thisEndPos.Y) {
		start.setPosition(start.position() + COORD{ 0, diff.Y });
		end.setPosition(end.position() + COORD{ 0, diff.Y });
		moved = true;
	}
}