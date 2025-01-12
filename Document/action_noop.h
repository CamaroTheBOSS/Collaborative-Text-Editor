#pragma once
#include "action.h"

class NoopAction : public Action {
public:
	NoopAction();
	bool tryMerge(const ActionPtr& other) override;
	COORD getStartPos() const override;
	COORD getEndPos() const override;
protected:
	void moveLeftCursor(const ActionPtr& other, const COORD& posDiff) override;
	void moveRightCursor(const ActionPtr& other, const COORD& posDiff) override;
	ModifyResult modify(const ActionPtr& other) override;
};