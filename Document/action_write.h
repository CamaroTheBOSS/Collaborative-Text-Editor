#pragma once
#include "action.h"
#include <unordered_map>

using ActionPtr = Action::ActionPtr;
using AffectPair = Action::AffectPair;

class WriteAction: public Action {
public:
	using Key = random::Engine::Key;
	WriteAction() = default;
	WriteAction(const COORD& startPos, std::vector<std::string>& text, TextContainer* target, Storage<ActionPtr>* eraseRegistry);
	WriteAction(const COORD& startPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, Storage<ActionPtr>* eraseRegistry);
	WriteAction(const COORD& startPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, const bool isChild, Storage<ActionPtr>* eraseRegistry);
	~WriteAction() override;

	AffectPair affect(Action& other) override;
	bool tryMerge(const ActionPtr& other) override;
	Action::UndoPair undo() override;
	COORD getLeftPos() const override;
	COORD getRightPos() const override;
	COORD getEndPos() const override;
protected:
	Key addToRegistry(const COORD& startPos, const COORD& endPos, TextContainer& text, Action& other);
	void removeItselfFromRegistry();
	AffectPair affectWrite(Action& other) override;
	AffectPair affectErase(Action& other) override;
	void move(const COORD& otherStartPos, const COORD& diff) override;
};