#pragma once
#include "action.h"

class EraseAction : public Action {
public:
	using ActionPtr = std::unique_ptr<Action>;

	EraseAction() = default;
	EraseAction(COORD& leftPos, COORD& endPos, std::vector<std::string>& text);
	EraseAction(COORD& leftPos, COORD& endPos, std::vector<std::string>& text, Timestamp timestamp);

	ActionPtr convertToOppositeAction() const override;
	std::optional<ActionPtr> affect(Action& other, const bool sameUser, const bool fromUndo) override;
	bool tryMerge(const ActionPtr& other) override;
	Action::UndoPair undo(const int userIdx, Document& doc) const override;
	COORD getLeftPos() const override;
	COORD getRightPos() const override;
	COORD getEndPos() const override;
private:
	void addUndoHook(ActionSptr& action, const int token) override;
	std::optional<ActionPtr> affectWrite(Action& other, const bool sameUser, const bool fromUndo) override;
	std::optional<ActionPtr> affectErase(Action& other, const bool sameUser, const bool fromUndo) override;
	void move(const COORD& otherStartPos, const COORD& diff) override;
	Cursor end;
	bool moved = false;
	std::vector<std::pair<ActionSptr, int>> undoHooks;
};