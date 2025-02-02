
#include "pch.h"
#include "action_history.h"
#include "pos_helpers.h"
#include "action_write.h"
#include "action_erase.h"

constexpr int historyLimit = 10;

ActionPtr makeWriteAction(COORD startPos, std::vector<std::string> txt) {
	return std::make_unique<WriteAction>(startPos, txt, nullptr, nullptr);
}

ActionPtr makeEmptyWriteAction() {
	return std::make_unique<WriteAction>();
}

ActionPtr makeEraseAction(COORD startPos, COORD endPos, std::vector<std::string> txt) {
	return std::make_unique<EraseAction>(startPos, endPos, txt, nullptr, nullptr);
}

ActionPtr makeEmptyEraseAction() {
	return std::make_unique<EraseAction>();
}

void addWriteActionToHistory(ActionHistory& history, COORD startPos, std::vector<std::string> txt) {
	auto action = makeWriteAction(std::move(startPos), std::move(txt));
	history.affect(action);
	history.push(action);
}

void addEraseActionToHistory(ActionHistory& history, COORD startPos, COORD endPos, std::vector<std::string> txt) {
	auto action = makeEraseAction(std::move(startPos), std::move(endPos), std::move(txt));
	history.affect(action);
	history.push(action);
}

bool validateAction(const ActionPtr& action, COORD desiredStartPos, COORD desiredEndPos, std::string desiredTxt) {
	EXPECT_EQ(action->getStartPos(), desiredStartPos);
	EXPECT_EQ(action->getEndPos(), desiredEndPos);
	EXPECT_EQ(action->getText(), desiredTxt);
	return action->getStartPos() == desiredStartPos && action->getEndPos() == desiredEndPos && action->getText() == desiredTxt;
}

void validUndoAction(ActionHistory& history, COORD desiredStartPos, COORD desiredEndPos, std::string desiredTxt) {
	auto action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_EQ(action->getStartPos(), desiredStartPos);
	EXPECT_EQ(action->getEndPos(), desiredEndPos);
	EXPECT_EQ(action->getText(), desiredTxt);
}

ActionHistory prepSimpleHistory(const int nEntries, const int mergeIntervalMs = 0) {
	ActionHistory history{ std::chrono::milliseconds(mergeIntervalMs), historyLimit };
	int limit = nEntries < 0 ? historyLimit : nEntries;
	for (SHORT i = 0; i < limit; i++) {
		auto action = makeWriteAction({ 1, i }, std::vector<std::string>{"test" + std::to_string(i)});
		history.push(action);
	}
	return history;
}

TEST(ActionHistoryTests, CircularPushingTest) {
	auto history = prepSimpleHistory(-1);
	auto& actions = history.getUndoActions();
	EXPECT_EQ(actions.size(), historyLimit);
	for (SHORT i = historyLimit; i < historyLimit + 3; i++) {
		auto action = makeWriteAction({ 1, i }, std::vector<std::string>{"newtest" + std::to_string(i)});
		history.push(action);
		EXPECT_EQ(actions.size(), historyLimit);
	}
	std::vector<std::string> expected{"test3", "test4", "test5"};
	for (int i = 0; i < 3; i++) {
		EXPECT_EQ(actions[i]->getText(), expected[i]);
	}
}

TEST(ActionHistoryTests, UndoTest) {
	constexpr int nEntries = 3;
	auto history = prepSimpleHistory(nEntries);
	for (int i = 0; i < nEntries; i++) {
		auto action = history.undo();
		EXPECT_EQ(std::move(action).value_or(makeEmptyWriteAction())->getText(), "test" + std::to_string(nEntries - i - 1));
	}
	auto action = history.undo();
	EXPECT_FALSE(action.has_value());
}

TEST(ActionHistoryTests, EmptyHistoryUndoTest) {
	constexpr int nEntries = 0;
	auto history = prepSimpleHistory(nEntries);
	auto action = history.undo();
	EXPECT_FALSE(action.has_value());
}

TEST(ActionHistoryTests, RedoTest) {
	constexpr int nEntries = 3;
	auto history = prepSimpleHistory(nEntries);
	auto action = history.redo();
	EXPECT_FALSE(action.has_value());
}

TEST(ActionHistoryTests, EmptyHistoryRedoTest) {
	constexpr int nEntries = 0;
	auto history = prepSimpleHistory(nEntries);
	auto action = history.redo();
	EXPECT_FALSE(action.has_value());
}

TEST(ActionHistoryTests, UndoRedoTest) {
	constexpr int nEntries = 3;
	auto history = prepSimpleHistory(nEntries);
	auto action = history.undo();
	EXPECT_EQ(action.value()->getText(), "test2");
	history.pushToRedo(action.value());
	action = history.redo();
	EXPECT_EQ(action.value()->getText(), "test2");
	action = history.redo();
	EXPECT_FALSE(action.has_value());
}

TEST(ActionHistoryTests, FullUndoRedoTest) {
	constexpr int nEntries = 3;
	auto history = prepSimpleHistory(nEntries);
	for (int i = 0; i < nEntries; i++) {
		auto action = history.undo();
		EXPECT_EQ(action.value()->getText(), "test" + std::to_string(nEntries - i - 1));
		history.pushToRedo(action.value());
	}
	for (int i = 0; i < nEntries; i++) {
		auto action = history.redo();
		EXPECT_EQ(action.value()->getText(), "test" + std::to_string(i));
	}
	auto action = history.redo();
	EXPECT_FALSE(action.has_value());
}

TEST(ActionHistoryTests, PushAfterUndoLoseRedoOptionTest) {
	constexpr int nEntries = 3;
	auto history = prepSimpleHistory(nEntries);
	auto action = history.undo();
	EXPECT_EQ(std::move(action).value_or(makeEmptyWriteAction())->getText(), "test2");
	auto actionToPush = makeWriteAction({ 1, 4 }, std::vector<std::string>{"testX"});
	history.push(actionToPush);
	action = history.redo();
	EXPECT_FALSE(action.has_value());
	action = history.undo();
	EXPECT_EQ(std::move(action).value_or(makeEmptyWriteAction())->getText(), "testX");
}

TEST(ActionHistoryTests, MergeWriteFromRightSideActionsTest) {
	constexpr int nEntries = 3;
	ActionHistory history{ std::chrono::milliseconds(10000000), historyLimit };
	for (int i = 0; i < nEntries; i++) {
		auto action = makeWriteAction(
			{ static_cast<SHORT>(5 * i), 0 },
			std::vector<std::string>{"test" + std::to_string(i)}
		);
		history.push(action);
	}
	auto action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_EQ(action->getText(), "test0test1test2");
	EXPECT_TRUE(action->getStartPos() == COORD(0, 0));
	EXPECT_TRUE(action->getEndPos() == COORD(15, 0));
}

TEST(ActionHistoryTests, DontMergeWriteFromLeftSideActionsTest) {
	constexpr int nEntries = 3;
	ActionHistory history{ std::chrono::milliseconds(10000000), historyLimit };
	for (int i = nEntries - 1; i >= 0; i--) {
		auto action = makeWriteAction(
			{ static_cast<SHORT>(5 * i), 0 },
			std::vector<std::string>{"test" + std::to_string(i)}
		);
		history.push(action);
	}
	EXPECT_EQ(history.getUndoActions().size(), nEntries);
	auto action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_EQ(action->getText(), "test0");
	EXPECT_TRUE(action->getStartPos() == COORD(0, 0));
	EXPECT_TRUE(action->getEndPos() == COORD(5, 0));
}

TEST(ActionHistoryTests, MergeEraseFromLeftSideActionsTest) {
	constexpr int nEntries = 3;
	ActionHistory history{ std::chrono::milliseconds(10000000), historyLimit };
	for (int i = nEntries; i > 0; i--) {
		auto action = makeEraseAction(
			{ static_cast<SHORT>(5 * i), 0 },
			{ static_cast<SHORT>(5 * (i - 1)), 0 },
			std::vector<std::string>{"test" + std::to_string(i)}
		);
		history.push(action);
	}
	EXPECT_EQ(history.getUndoActions().size(), 1);
	auto action = history.undo().value_or(makeEmptyEraseAction());
	EXPECT_EQ(action->getText(), "test1test2test3");
	EXPECT_TRUE(action->getStartPos() == COORD(15, 0));
	EXPECT_TRUE(action->getEndPos() == COORD(0, 0));
}

TEST(ActionHistoryTests, DontMergeEraseFromRightSideActionsTest) {
	constexpr int nEntries = 3;
	ActionHistory history{ std::chrono::milliseconds(10000000), historyLimit };
	for (int i = 1; i <= nEntries; i++) {
		auto action = makeEraseAction(
			{ static_cast<SHORT>(5 * i), 0 },
			{ static_cast<SHORT>(5 * (i - 1)), 0 },
			std::vector<std::string>{"test" + std::to_string(i)}
		);
		history.push(action);
	}
	EXPECT_EQ(history.getUndoActions().size(), 3);
	auto action = history.undo().value_or(makeEmptyEraseAction());
	EXPECT_EQ(action->getText(), "test3");
	EXPECT_TRUE(action->getStartPos() == COORD(15, 0));
	EXPECT_TRUE(action->getEndPos() == COORD(10, 0));
}

TEST(ActionHistoryTests, SplitActionTest) {
	// ...[...(...)...]... //
	ActionHistory history{ std::chrono::milliseconds(0), historyLimit };
	auto action = makeWriteAction(COORD{ 0, 0 }, std::vector<std::string>{"verylongtest"});
	history.push(action);

	auto anotherUserAction = makeWriteAction(COORD{ 4, 0 }, std::vector<std::string>{"test"});
	history.affect(anotherUserAction);

	EXPECT_EQ(history.getUndoActions().size(), 2);
	action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_TRUE(action->getStartPos() == COORD(8, 0));
	EXPECT_TRUE(action->getEndPos() == COORD(16, 0));
	EXPECT_EQ(action->getText(), "longtest");
	action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_TRUE(action->getStartPos() == COORD(0, 0));
	EXPECT_TRUE(action->getEndPos() == COORD(4, 0));
	EXPECT_EQ(action->getText(), "very");
}

TEST(ActionHistoryTests, SplitActionWithEndlTest) {
	// ...[...(...)...]... //
	ActionHistory history{ std::chrono::milliseconds(0), historyLimit };
	auto action = makeWriteAction(COORD{ 0, 0 }, std::vector<std::string>{"verylongtest", "xd"});
	history.push(action);

	auto anotherUserAction = makeWriteAction(COORD{ 4, 0 }, std::vector<std::string>{"ThisTextIsSoLong", ""});
	COORD diffPos = COORD{ -4, 1 };
	history.affect(anotherUserAction);

	EXPECT_EQ(history.getUndoActions().size(), 2);
	action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_TRUE(action->getStartPos() == COORD(0, 1));
	EXPECT_TRUE(action->getEndPos() == COORD(2, 2));
	EXPECT_EQ(action->getText(), "longtest\nxd");
	action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_TRUE(action->getStartPos() == COORD(0, 0));
	EXPECT_TRUE(action->getEndPos() == COORD(4, 0));
	EXPECT_EQ(action->getText(), "very");
}

TEST(ActionHistoryTests, a1bcTest) {
	ActionHistory history{ std::chrono::milliseconds(0), historyLimit };
	addWriteActionToHistory(history, COORD{ 0, 0 }, std::vector<std::string>{"a"});
	addWriteActionToHistory(history, COORD{ 1, 0 }, std::vector<std::string>{"b"});
	addWriteActionToHistory(history, COORD{ 2, 0 }, std::vector<std::string>{"c"});
	addWriteActionToHistory(history, COORD{ 1, 0 }, std::vector<std::string>{"1"});
	history.undo();
	auto action = makeEraseAction(COORD{ 2, 0 }, COORD{ 1, 0 }, std::vector<std::string>{"1"});
	history.affect(action);

	auto& actions = history.getUndoActions();
	EXPECT_EQ(actions.size(), 3);
	if (actions.size() == 3) {
		validateAction(actions[0], COORD{0, 0}, COORD{1, 0}, "a");
		validateAction(actions[1], COORD{ 1, 0 }, COORD{ 2, 0 }, "b");
		validateAction(actions[2], COORD{ 2, 0 }, COORD{ 3, 0 }, "c");
	}
}

TEST(ActionHistoryTests, ActionsWithEndlinesSplitTest) {
	ActionHistory history1{ std::chrono::milliseconds(0), historyLimit };
	addWriteActionToHistory(history1, COORD{ 0, 0 }, std::vector<std::string>{"first", "second"});
	addWriteActionToHistory(history1, COORD{ 6, 1 }, std::vector<std::string>{"", "third", "fourth"});
	addWriteActionToHistory(history1, COORD{ 6, 3 }, std::vector<std::string>{"", "fifth"});
	auto action = makeWriteAction(COORD{ 2, 4 }, std::vector<std::string>{"1234"});
	history1.affect(action);
	action = makeWriteAction(COORD{ 6, 3 }, std::vector<std::string>{"123"});
	history1.affect(action);
	auto& actions = history1.getUndoActions();
	EXPECT_EQ(actions.size(), 4);
	if (actions.size() == 4) {
		validateAction(actions[0], COORD{ 0, 0 }, COORD{ 6, 1 }, "first\nsecond");
		validateAction(actions[1], COORD{ 6, 1 }, COORD{ 6, 3 }, "\nthird\nfourth");
		validateAction(actions[2], COORD{ 9, 3 }, COORD{ 2, 4 }, "\nfi");
		validateAction(actions[3], COORD{ 6, 4 }, COORD{ 9, 4 }, "fth");
	}
}

TEST(ActionHistoryTests, ActionsWithEndlinesNewActionWithEndlineSplitTest) {
	ActionHistory history1{ std::chrono::milliseconds(0), historyLimit };
	ActionHistory history2{ std::chrono::milliseconds(0), historyLimit };
	addWriteActionToHistory(history1, COORD{ 0, 0 }, std::vector<std::string>{"first", "second"});
	addWriteActionToHistory(history1, COORD{ 6, 1 }, std::vector<std::string>{"", "third", "fourth"});
	addWriteActionToHistory(history1, COORD{ 6, 3 }, std::vector<std::string>{"", "fifth"});
	auto action = makeWriteAction(COORD{ 2, 4 }, std::vector<std::string>{"1234", ""});
	history1.affect(action);
	auto& actions = history1.getUndoActions();
	EXPECT_EQ(actions.size(), 4);
	if (actions.size() == 4) {
		validateAction(actions[0], COORD{ 0, 0 }, COORD{ 6, 1 }, "first\nsecond");
		validateAction(actions[1], COORD{ 6, 1 }, COORD{ 6, 3 }, "\nthird\nfourth");
		validateAction(actions[2], COORD{ 6, 3 }, COORD{ 2, 4 }, "\nfi");
		validateAction(actions[3], COORD{ 0, 5 }, COORD{ 3, 5 }, "fth");
	}
}

TEST(ActionHistoryTests, CutActionFromRightTest) {
	// ...[...(...]...)... //
	ActionHistory history{ std::chrono::milliseconds(0), historyLimit };
	addWriteActionToHistory(history, COORD{ 0, 0 }, std::vector<std::string>{"testing123"});

	auto action = makeEraseAction(COORD{ 12, 0 }, COORD{ 8, 0 }, std::vector<std::string>{"23xd"});
	history.affect(action);

	auto& actions = history.getUndoActions();
	EXPECT_EQ(actions.size(), 1);
	if (actions.size() == 1) {
		validateAction(actions[0], COORD{0, 0}, COORD{8, 0}, "testing1");
	}
}

TEST(ActionHistoryTests, CutActionFromLeftTest) {
	// ...[...(...]...)... //
	ActionHistory history{ std::chrono::milliseconds(0), historyLimit };
	addWriteActionToHistory(history, COORD{ 2, 0 }, std::vector<std::string>{"testing123"});

	auto action = makeEraseAction(COORD{ 4, 0 }, COORD{ 0, 0 }, std::vector<std::string>{"xdte"});
	history.affect(action);

	auto& actions = history.getUndoActions();
	EXPECT_EQ(actions.size(), 1);
	if (actions.size() == 1) {
		validateAction(actions[0], COORD{0, 0}, COORD{8, 0}, "sting123");
	}
}

// ----- NOT RELEVANT ------
TEST(ActionHistoryTests, DeleteActionTest) {
	// ...[(...)]... //
	ActionHistory history{ std::chrono::milliseconds(0), historyLimit };
	addWriteActionToHistory(history, COORD{ 2, 0 }, std::vector<std::string>{"testing123"});

	auto action = makeEraseAction(COORD{ 12, 0 }, COORD{ 2, 0 }, std::vector<std::string>{"testing123"});
	history.affect(action);

	auto& actions = history.getUndoActions();
	EXPECT_EQ(actions.size(), 1);
	if (actions.size() == 1) {
		EXPECT_EQ(actions[0]->getText(), "");
	}
}

TEST(ActionHistoryTests, EraseInWriteTest) {
	// ...[...(...]...)... //
	ActionHistory history{ std::chrono::milliseconds(0), historyLimit };
	addWriteActionToHistory(history, COORD{ 2, 0 }, std::vector<std::string>{"testing123"});

	auto action = makeEraseAction(COORD{ 8, 0 }, COORD{ 4, 0 }, std::vector<std::string>{"stin"});
	history.affect(action);

	auto& actions = history.getUndoActions();
	EXPECT_EQ(actions.size(), 1);
	if (actions.size() == 1) {
		validateAction(actions[0], COORD{2, 0}, COORD{8, 0}, "teg123");
	}
}