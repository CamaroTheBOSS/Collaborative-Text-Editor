#pragma once
#include "action.h"

using ActionPtr = Action::ActionPtr;
using AffectPair = Action::AffectPair;

class EraseAction : public Action {
public:
	EraseAction() = default;
	EraseAction(const COORD& startPos, const COORD& endPos, std::vector<std::string>& text, TextContainer* target, Storage<ActionPtr>* eraseRegistry);
	EraseAction(const COORD& startPos, const COORD& endPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, Storage<ActionPtr>* eraseRegistry);
	EraseAction(const COORD& startPos, const COORD& endPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, const bool isChild, Storage<ActionPtr>* eraseRegistry);

	AffectPair affect(Action& other) override;
	bool tryMerge(const ActionPtr& other) override;
	Action::UndoPair undo() override;
	COORD getLeftPos() const override;
	COORD getRightPos() const override;
	COORD getEndPos() const override;
	void triggerRelatedActions() override;
protected:
	AffectPair affectWrite(Action& other) override;
	AffectPair affectErase(Action& other) override;
	void move(const COORD& otherStartPos, const COORD& diff) override;

	Cursor end;
};