#include "action_write.h"
#include "pos_helpers.h"
#include "document.h"
#include "action_erase.h"
#include "parser.h"

using ActionPtr = WriteAction::ActionPtr;

WriteAction::WriteAction(COORD& startPos, std::vector<std::string>& text):
	Action(ActionType::write, startPos, text) {}

WriteAction::WriteAction(COORD& startPos, std::vector<std::string>& text, Timestamp timestamp):
	Action(ActionType::write, startPos, text, timestamp) {}

ActionPtr WriteAction::convertToOppositeAction() const {
	COORD startPos = getEndPos();
	COORD endPos = getStartPos();
	auto textCopy = text;
	return std::make_unique<EraseAction>(startPos, endPos, textCopy, getTimestamp());
}

std::optional<ActionPtr> WriteAction::affect(const ActionPtr& other) {
	return other->affectWrite(*this);
}

std::optional<ActionPtr> WriteAction::affectWrite(const Action& other) {
	COORD thisLeft = getLeftPos();
	COORD thisRight = getRightPos();
	COORD otherLeft = other.getLeftPos();
	COORD otherRight = other.getRightPos();
	if (otherLeft >= thisRight) {
		return {};
	}
	else if (otherLeft <= thisLeft) {
		move(otherLeft, otherRight - otherLeft);
		return {};
	}
	COORD splitPoint = positionalDiff(otherLeft, thisLeft);
	auto splittedText = splitText(splitPoint);
	auto newAction = std::make_unique<WriteAction>(otherRight, splittedText, getTimestamp());
	return std::make_optional<ActionPtr>(std::move(newAction));
}

std::optional<ActionPtr> WriteAction::affectErase(const Action& other) {
	COORD thisLeft = getLeftPos();
	COORD thisRight = getRightPos();
	COORD otherLeft = other.getLeftPos();
	COORD otherRight = other.getRightPos();
	if (otherLeft >= thisRight) {
		return {};
	}
	//Update text if needed
	COORD rightSplitPoint = positionalDiff(otherRight, thisLeft);
	COORD leftSplitPoint = positionalDiff(otherLeft, thisLeft);
	auto rightSplittedText = splitText(rightSplitPoint);
	auto middleSplittedText = splitText(leftSplitPoint);
	if (thisLeft <= otherRight) {
		text = thisLeft >= otherLeft ? rightSplittedText : mergeText(text, rightSplittedText);
	}

	//Move if needed
	if (thisLeft > otherLeft) {
		int x = otherRight.Y >= thisLeft.Y ? 
			otherLeft.X + (std::max)(thisLeft - otherRight, COORD{0, 0}).X :
			thisLeft.X;
		int y = otherRight.Y < thisLeft.Y ?
			thisLeft.Y - (otherRight.Y - otherLeft.Y) :
			otherLeft.Y;
		start.setPosition(COORD{ static_cast<SHORT>(x), static_cast<SHORT>(y) });
	}
	return {};
}

bool WriteAction::tryMerge(const ActionPtr& other) {
	if (dynamic_cast<WriteAction*>(other.get()) == nullptr || getEndPos() != other->getStartPos()) {
		return false;
	}
	text = std::move(mergeText(text, other->text));
	return true;
}

UndoReturn WriteAction::undo(const int userIdx, Document& doc) const {
	COORD startPos = (std::min)(doc.getEndPos(), getEndPos());
	std::vector<std::string> erasedText;
	COORD endPos = doc.eraseText(startPos, getText().size(), erasedText);
	COORD diffPos = endPos - startPos;
	doc.moveAffectedCursors(doc.users[userIdx], diffPos);
	doc.adjustCursors();
	return {ActionType::erase, startPos, endPos, Parser::parseVectorToText(erasedText)};
}

COORD WriteAction::getLeftPos() const {
	return getStartPos();
}

COORD WriteAction::getRightPos() const {
	COORD textSize = getTextSize();
	if (textSize.Y <= 1) {
		return start.position() + COORD{textSize.X, 0};
	}
	return COORD{ textSize.X, static_cast<SHORT>(start.position().Y + textSize.Y - 1) };
}

COORD WriteAction::getEndPos() const {
	return getRightPos();
}

void WriteAction::move(const COORD& otherStartPos, const COORD& diff) {
	auto thisPos = getStartPos();
	if (otherStartPos.Y == thisPos.Y && otherStartPos.X <= thisPos.X) {
		start.setPosition(start.position() + diff);
	}
	else if (otherStartPos.Y < thisPos.Y) {
		start.setPosition(start.position() + COORD{ 0, diff.Y });
	}
}
