#include "action.h"
#include "pos_helpers.h"

Action::Action(const ActionType type, const COORD& startPos, std::vector<std::string>& text, TextContainer* target, Storage<ActionPtr>* eraseRegistry) :
	type(type),
	start(Cursor(startPos)),
	container(text),
	target(target),
	timestamp(std::chrono::system_clock::now()),
	eraseRegistry(eraseRegistry) {}

Action::Action(const ActionType type, const COORD& startPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, Storage<ActionPtr>* eraseRegistry):
	type(type),
	start(Cursor(startPos)),
	container(std::move(text)),
	target(target),
	timestamp(timestamp),
	eraseRegistry(eraseRegistry) {}

Action::Action(const ActionType type, const COORD& startPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, const bool isChild, Storage<ActionPtr>* eraseRegistry) :
	type(type),
	child(isChild),
	start(Cursor(startPos)),
	container(std::move(text)),
	target(target),
	timestamp(timestamp),
	eraseRegistry(eraseRegistry) {}

bool Action::empty() const {
	return container.empty();
}

bool Action::isChild() const {
	return child;
}

ActionType Action::getType() const {
	return type;
}

COORD Action::getStartPos() const {
	return start.position();
}

std::string Action::getText() const {
	return container.getText();
}

Timestamp Action::getTimestamp() const {
	return timestamp;
}

void Action::addRelationship(const Key key) {
	relationshipRegistry.push_back(key);
}
