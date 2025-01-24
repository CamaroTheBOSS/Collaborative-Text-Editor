#pragma once
#include "action.h"

class EraseAction : public Action {
public:
	using ActionPtr = std::shared_ptr<Action>;

	EraseAction() = default;
	EraseAction(COORD& leftPos, COORD& endPos, std::vector<std::string>& text);
	EraseAction(COORD& leftPos, COORD& endPos, std::vector<std::string>& text, Timestamp timestamp);

	std::optional<ActionPtr> affect(const ActionPtr& other) override;
	std::optional<ActionPtr> affectWrite(const Action& other) override;
	std::optional<ActionPtr> affectErase(const Action& other) override;
	bool tryMerge(const ActionPtr& other) override;
	COORD getLeftPos() const override;
	COORD getRightPos() const override;
	COORD getEndPos() const override;
private:
	void move(const COORD& otherStartPos, const COORD& diff) override;
	Cursor end; //only needed when new EraseAction is created and it is used in affect part (after that it is not updated cause its not possible to track this position correctly)
	bool moved = false;
};