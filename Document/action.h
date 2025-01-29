#pragma once
#include <chrono>
#include <vector>
#include <string>
#include <optional>

#include "cursor.h"

using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
enum class ActionType {write, erase, noop};

struct UndoReturn {
	ActionType type;
	COORD startPos;
	COORD endPos;
	std::string text;
};

class Document;
class Action {
public:
	using ActionPtr = std::unique_ptr<Action>;
	using UndoPair = std::pair<ActionPtr, UndoReturn>;
	friend class WriteAction;
	friend class EraseAction;

	Action() = default;
	Action(ActionType type, COORD& leftPos, std::vector<std::string>& text);
	Action(ActionType type, COORD& leftPos, std::vector<std::string>& text, Timestamp timestamp);
	
	virtual ActionPtr convertToOppositeAction() const = 0;
	virtual std::optional<ActionPtr> affect(Action& other) const = 0;
	virtual bool tryMerge(const ActionPtr& other) = 0;
	virtual UndoPair undo(const int userIdx, Document& doc) const = 0;
	virtual COORD getLeftPos() const = 0;
	virtual COORD getRightPos() const = 0;
	virtual COORD getEndPos() const = 0;
	COORD getTextSize() const;
	COORD getStartPos() const;
	ActionType getType() const;
	
	std::string getText() const;
	Timestamp getTimestamp() const;
	std::vector<std::string> splitText(const COORD& splitPoint);
	std::vector<std::string>& mergeText(std::vector<std::string>& firstText, std::vector<std::string>& otherText);
protected:
	virtual std::optional<ActionPtr> affectWrite(const Action& other) = 0;
	virtual std::optional<ActionPtr> affectErase(const Action& other) = 0;
	virtual void move(const COORD& otherStartPos, const COORD& diff) = 0;

	ActionType type{ActionType::noop};
	Cursor start;
	std::vector<std::string> text;
	Timestamp timestamp;
};