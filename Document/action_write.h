#pragma once
#include "action.h"

class WriteAction : public Action {
public:
	WriteAction() = default;
	WriteAction(COORD&& startPos, COORD&& endPos, std::string&& txt);
	bool tryMerge(const ActionPtr& other) override;
	COORD getStartPos() const override;
	COORD getEndPos() const override;
protected:
	void moveLeftCursor(const ActionPtr& other, const COORD& posDiff) override;
	void moveRightCursor(const ActionPtr& other, const COORD& posDiff) override;
	ModifyResult modify(const ActionPtr& other) override;
	ModifyResult modifyErase(const ActionPtr& other);
	ModifyResult modifyWrite(const ActionPtr& other);
};