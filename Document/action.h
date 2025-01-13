#pragma once
#include <chrono>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include "cursor.h"

using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
enum class ActionType { write, erase, replace, noop };
class Action {
public:
	friend class WriteAction;
	friend class EraseAction;
	Action() = default;
	Action(ActionType&& type, COORD&& startPos, COORD&& endPos, std::string&& txt);
	using ActionPtr = std::shared_ptr<Action>;
	using ModifyResult = std::pair<bool, std::optional<ActionPtr>>;
	virtual bool tryMerge(const ActionPtr& other) = 0;
	virtual COORD getStartPos() const = 0;
	virtual COORD getEndPos() const = 0;

	ModifyResult affect(const ActionPtr& other, const COORD& posDiff);
	void setLeft(const COORD newPos);
	void setRight(const COORD newPos);
	COORD getLeft() const;
	COORD getRight() const;
	ActionType getType() const;
	std::string getText() const;
	Timestamp getTimestamp() const;

protected:
	virtual void moveLeftCursor(const ActionPtr& other, const COORD& posDiff) = 0;
	virtual void moveRightCursor(const ActionPtr& other, const COORD& posDiff) = 0;
	virtual ModifyResult modify(const ActionPtr& other) = 0;

	ActionType type;
	std::string text;
	Timestamp timestamp;
	Cursor left;
	Cursor right;
};