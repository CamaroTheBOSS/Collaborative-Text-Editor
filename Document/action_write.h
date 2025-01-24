#pragma once
#include "action.h"

class WriteAction: public Action {
public:
	using ActionPtr = std::shared_ptr<Action>;

	WriteAction() = default;
	WriteAction(COORD& leftPos, std::vector<std::string>& text);
	WriteAction(COORD& leftPos, std::vector<std::string>& text, Timestamp timestamp);

	std::optional<ActionPtr> affect(const ActionPtr& other) override;
	std::optional<ActionPtr> affectWrite(const Action& other) override;
	std::optional<ActionPtr> affectErase(const Action& other) override;
	bool tryMerge(const ActionPtr& other) override;
	COORD getLeftPos() const override;
	COORD getRightPos() const override;
	COORD getEndPos() const override;
private:
	void move(const COORD& otherStartPos, const COORD& diff) override;
};