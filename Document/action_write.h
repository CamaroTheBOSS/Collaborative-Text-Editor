#pragma once
#include "action.h"
#include <unordered_map>

class WriteAction: public Action {
public:
	using ActionPtr = std::unique_ptr<Action>;

	WriteAction() = default;
	WriteAction(COORD& leftPos, std::vector<std::string>& text);
	WriteAction(COORD& leftPos, std::vector<std::string>& text, Timestamp timestamp);

	ActionPtr convertToOppositeAction() const override;
	std::optional<ActionPtr> affect(Action& other, const bool sameUser, const bool fromUndo) override;
	bool tryMerge(const ActionPtr& other) override;
	Action::UndoPair undo(const int userIdx, Document& doc) const override;
	COORD getLeftPos() const override;
	COORD getRightPos() const override;
	COORD getEndPos() const override;
private:
	void addToHistory(Action& other, COORD pos, std::vector<std::string>& text);
	void addUndoHook(ActionSptr& action, const int token) override;
	std::optional<ActionPtr> affectWrite(Action& other, const bool sameUser, const bool fromUndo) override;
	std::optional<ActionPtr> affectErase(Action& other, const bool sameUser, const bool fromUndo) override;
	void move(const COORD& otherStartPos, const COORD& diff) override;

	std::unordered_map<int, ActionSptr> history;
	int revisionIndex = 0;
};