#pragma once
#include <chrono>
#include <vector>
#include <string>
#include <optional>

#include "cursor.h"

using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

class Action {
public:
	using ActionPtr = std::shared_ptr<Action>;
	friend class WriteAction;
	friend class EraseAction;

	Action() = default;
	Action(COORD& leftPos, std::vector<std::string>& text);
	Action(COORD& leftPos, std::vector<std::string>& text, Timestamp timestamp);
	
	virtual std::optional<ActionPtr> affect(const ActionPtr& other) = 0;
	virtual std::optional<ActionPtr> affectWrite(const Action& other) = 0;
	virtual std::optional<ActionPtr> affectErase(const Action& other) = 0;
	virtual bool tryMerge(const ActionPtr& other) = 0;
	virtual COORD getLeftPos() const = 0;
	virtual COORD getRightPos() const = 0;
	virtual COORD getEndPos() const = 0;
	COORD getTextSize() const;
	COORD getStartPos() const;
	
	
	std::string getText() const;
	Timestamp getTimestamp() const;
	std::vector<std::string> splitText(const COORD& splitPoint);
	std::vector<std::string>& mergeText(std::vector<std::string>& firstText, std::vector<std::string>& otherText);
protected:
	virtual void move(const COORD& otherStartPos, const COORD& diff) = 0;

	Cursor start;
	std::vector<std::string> text;
	Timestamp timestamp;
};