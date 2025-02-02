#pragma once
#include <chrono>
#include <vector>
#include <string>
#include <optional>

#include "storage.h"
#include "cursor.h"
#include "engine.h"
#include "text_container.h"

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
	using Key = random::Engine::Key;
	using ActionPtr = std::unique_ptr<Action>;
	using UndoPair = std::pair<ActionPtr, UndoReturn>;
	using AffectPair = std::pair<std::optional<ActionPtr>, std::optional<Key>>;
	friend class WriteAction;
	friend class EraseAction;

	Action() = default;
	Action(const ActionType type, const COORD& startPos, std::vector<std::string>& text, TextContainer* target, Storage<ActionPtr>* eraseRegistry);
	Action(const ActionType type, const COORD& startPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, Storage<ActionPtr>* eraseRegistry);
	virtual ~Action() = default;

	virtual AffectPair affect(Action& other) = 0;
	virtual bool tryMerge(const ActionPtr& other) = 0;
	virtual UndoPair undo() = 0;
	virtual COORD getLeftPos() const = 0;
	virtual COORD getRightPos() const = 0;
	virtual COORD getEndPos() const = 0;
	virtual void triggerRelatedActions() {};

	bool empty() const;
	bool isChild() const;
	COORD getStartPos() const;
	ActionType getType() const;
	std::string getText() const;
	Timestamp getTimestamp() const;
	void addRelationship(const Key key);
protected:
	Action(const ActionType type, const COORD& startPos, TextContainer& text, TextContainer* target, const Timestamp timestamp, const bool isChild, Storage<ActionPtr>* eraseRegistry);

	virtual AffectPair affectWrite(Action& other) = 0;
	virtual AffectPair affectErase(Action& other) = 0;
	virtual void move(const COORD& otherStartPos, const COORD& diff) = 0;

	bool child = false; // Property which says that this action was created by another action during undo
	ActionType type{ActionType::noop};
	Cursor start;
	Timestamp timestamp;
	TextContainer container;
	TextContainer* target{ nullptr };
	Storage<ActionPtr>* eraseRegistry{ nullptr };
	std::vector<Key> relationshipRegistry;
};