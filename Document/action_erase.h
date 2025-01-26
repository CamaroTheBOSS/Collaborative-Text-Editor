#pragma once
#include "action.h"

class EraseAction : public Action {
public:
	using ActionPtr = std::unique_ptr<Action>;

	EraseAction() = default;
	EraseAction(COORD& leftPos, COORD& endPos, std::vector<std::string>& text);
	EraseAction(COORD& leftPos, COORD& endPos, std::vector<std::string>& text, Timestamp timestamp);

	ActionPtr convertToOppositeAction() const override;
	std::optional<ActionPtr> affect(const ActionPtr& other) override;
	bool tryMerge(const ActionPtr& other) override;
	UndoReturn undo(const int userIdx, Document& doc) const override;
	COORD getLeftPos() const override;
	COORD getRightPos() const override;
	COORD getEndPos() const override;
private:
	std::optional<ActionPtr> affectWrite(const Action& other) override;
	std::optional<ActionPtr> affectErase(const Action& other) override;
	void move(const COORD& otherStartPos, const COORD& diff) override;
	Cursor end;
	bool moved = false;
};