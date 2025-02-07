#include "action_write.h"
#include "action_erase.h"
#include "pos_helpers.h"

WriteAction::WriteAction(const COORD& startPos, std::vector<std::string>& text, TextContainer* target, Storage<ActionPtr>* eraseRegistry):
	Action(ActionType::write, startPos, text, target, eraseRegistry) {}

WriteAction::WriteAction(const COORD& startPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, Storage<ActionPtr>* eraseRegistry):
	Action(ActionType::write, startPos, text, target, timestamp, eraseRegistry) {}

WriteAction::WriteAction(const COORD& startPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, const bool isChild, Storage<ActionPtr>* eraseRegistry):
	Action(ActionType::write, startPos, text, target, timestamp, isChild, eraseRegistry) {}

AffectPair WriteAction::affect(Action& other) {
	return other.affectWrite(*this);
}

WriteAction::~WriteAction() {
	removeItselfFromRegistry();
}

AffectPair WriteAction::affectWrite(Action& other) {
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
	auto splittedText = container.split(splitPoint);
	auto newAction = std::make_unique<WriteAction>(otherRight, splittedText, target, getTimestamp(), eraseRegistry);
	return std::make_pair(std::move(newAction), std::optional<Key>{});
}

AffectPair WriteAction::affectErase(Action& other) {
	COORD thisLeft = getLeftPos();
	COORD thisRight = getRightPos();
	COORD otherLeft = other.getLeftPos();
	COORD otherRight = other.getRightPos();
	if (otherLeft >= thisRight) {
		return {};
	}
	//Update text if needed
	COORD rightSplitPoint = (std::min)(positionalDiff(otherRight, thisLeft), container.getEndPos());
	COORD leftSplitPoint = (std::max)(positionalDiff(otherLeft, thisLeft), container.getStartPos());
	if (thisLeft <= otherRight && rightSplitPoint > leftSplitPoint) {
		std::vector<std::string> splittedText;
		container.eraseBetween(leftSplitPoint, rightSplitPoint, splittedText);
		TextContainer splittedContainer{ splittedText };
		addToRegistry(rightSplitPoint, leftSplitPoint, splittedContainer, other);
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
	if (other->getType() != ActionType::write || getEndPos() != other->getStartPos()) {
		return false;
	}
	container = container.merge(other->container);
	return true;
}

Action::UndoPair WriteAction::undo() {
	if (target == nullptr) {
		return {};
	}
	std::vector<std::string> erasedText;
	COORD startPos = target->validatePos(getEndPos());
	COORD endPos = target->erase(startPos, getText().size(), erasedText);
	UndoReturn undoReturn{ ActionType::erase, startPos, endPos, container.getText()};
	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, container, target, getTimestamp(), true, eraseRegistry);
	removeItselfFromRegistry();
	return std::make_pair<ActionPtr, UndoReturn>(std::move(action), std::move(undoReturn));
}

COORD WriteAction::getLeftPos() const {
	return getStartPos();
}

COORD WriteAction::getRightPos() const {
	COORD textSize = container.getSize();
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

WriteAction::Key WriteAction::addToRegistry(const COORD& startPos, const COORD& endPos, TextContainer& text, Action& other) {
	if (eraseRegistry == nullptr) {
		return random::Engine::get().getInvalidKey();
	}
	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, text, &container, getTimestamp(), eraseRegistry);
	Key key = eraseRegistry->push(action);
	other.addRelationship(key);
	addRelationship(key);
	return key;
}

void WriteAction::removeItselfFromRegistry() {
	eraseRegistry->erase(relationshipRegistry);
	relationshipRegistry.clear();
}
