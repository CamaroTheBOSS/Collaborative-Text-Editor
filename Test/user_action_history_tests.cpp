#include "pch.h"
#include "user_action_history.h"
#include "pos_helpers.h"
#include "action_write.h"
#include "action_erase.h"

ActionPtr makeWriteAction(COORD&& startPos, COORD&& endPos, std::string&& txt) {
	return std::make_shared<WriteAction>(std::move(startPos), std::move(endPos), std::move(txt));
}

ActionPtr makeEmptyWriteAction() {
	return std::make_shared<WriteAction>();
}

ActionPtr makeEraseAction(COORD&& startPos, COORD&& endPos, std::string&& txt) {
	return std::make_shared<EraseAction>(std::move(startPos), std::move(endPos), std::move(txt));
}

ActionPtr makeEmptyEraseAction() {
	return std::make_shared<EraseAction>();
}

UserActionHistory prepSimpleHistory(const int nEntries, const int mergeIntervalMs = 0) {
	UserActionHistory history{ std::chrono::milliseconds(mergeIntervalMs) };
	int historyLimit = nEntries < 0 ? history.getHistoryLimit() : nEntries;
	for (SHORT i = 0; i < historyLimit; i++) {
		auto action = makeWriteAction({ 1, i }, { 6, i }, std::string{"test" + std::to_string(i)});
		history.push(action);
	}
	return history;
}

TEST(UserActionHistoryTests, CircularPushingTest) {
	auto history = prepSimpleHistory(-1);
	int historyLimit = history.getHistoryLimit();
	auto& actions = history.getUndoActions();
	EXPECT_EQ(actions.size(), historyLimit);
	for (SHORT i = historyLimit; i < historyLimit + 3; i++) {
		auto action = makeWriteAction({ 1, i }, { 9, i }, std::string{"newtest" + std::to_string(i)});
		history.push(action);
		EXPECT_EQ(actions.size(), historyLimit);
	}
	EXPECT_EQ(actions[0]->getText(), "test3");
	EXPECT_EQ(actions[1]->getText(), "test4");
	EXPECT_EQ(actions[2]->getText(), "test5");
}

TEST(UserActionHistoryTests, UndoTest) {
	constexpr int nEntries = 3;
	auto history = prepSimpleHistory(nEntries);
	for (int i = 0; i < nEntries; i++) {
		auto action = history.undo();
		EXPECT_EQ(action.value_or(makeEmptyWriteAction())->getText(), "test" + std::to_string(nEntries - i - 1));
	}
	auto action = history.undo();
	EXPECT_FALSE(action.has_value());
}

TEST(UserActionHistoryTests, EmptyHistoryUndoTest) {
	constexpr int nEntries = 0;
	auto history = prepSimpleHistory(nEntries);
	auto action = history.undo();
	EXPECT_FALSE(action.has_value());
}

TEST(UserActionHistoryTests, RedoTest) {
	constexpr int nEntries = 3;
	auto history = prepSimpleHistory(nEntries);
	auto action = history.redo();
	EXPECT_FALSE(action.has_value());
}

TEST(UserActionHistoryTests, EmptyHistoryRedoTest) {
	constexpr int nEntries = 0;
	auto history = prepSimpleHistory(nEntries);
	auto action = history.redo();
	EXPECT_FALSE(action.has_value());
}

TEST(UserActionHistoryTests, UndoRedoTest) {
	constexpr int nEntries = 3;
	auto history = prepSimpleHistory(nEntries);
	auto action = history.undo();
	EXPECT_EQ(action.value_or(makeEmptyWriteAction())->getText(), "test2");
	action = history.redo();
	EXPECT_EQ(action.value_or(makeEmptyWriteAction())->getText(), "test2");
	action = history.redo();
	EXPECT_FALSE(action.has_value());
}

TEST(UserActionHistoryTests, FullUndoRedoTest) {
	constexpr int nEntries = 3;
	auto history = prepSimpleHistory(nEntries);
	for (int i = 0; i < nEntries; i++) {
		auto action = history.undo();
		EXPECT_EQ(action.value_or(makeEmptyWriteAction())->getText(), "test" + std::to_string(nEntries - i - 1));
	}
	for (int i = 0; i < nEntries; i++) {
		auto action = history.redo();
		EXPECT_EQ(action.value_or(makeEmptyWriteAction())->getText(), "test" + std::to_string(i));
	}
	auto action = history.redo();
	EXPECT_FALSE(action.has_value());
}

TEST(UserActionHistoryTests, PushAfterUndoLoseRedoOptionTest) {
	constexpr int nEntries = 3;
	auto history = prepSimpleHistory(nEntries);
	auto action = history.undo();
	EXPECT_EQ(action.value_or(makeEmptyWriteAction())->getText(), "test2");
	auto actionToPush = makeWriteAction({ 1, 4 }, { 6, 4 }, "testX");
	history.push(actionToPush);
	action = history.redo();
	EXPECT_FALSE(action.has_value());
	action = history.undo();
	EXPECT_EQ(action.value_or(makeEmptyWriteAction())->getText(), "testX");
}

TEST(UserActionHistoryTests, MergeWriteFromRightSideActionsTest) {
	constexpr int nEntries = 3;
	UserActionHistory history{ std::chrono::milliseconds(10000000) };
	for (int i = 0; i < nEntries; i++) {
		auto action = makeWriteAction(
			{ static_cast<SHORT>(5 * i), 0 },
			{ static_cast<SHORT>(5 * (i + 1)), 0 },
			std::string{"test" + std::to_string(i)}
		);
		history.push(action);
	}
	auto action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_EQ(action->getText(), "test0test1test2");
	EXPECT_TRUE(equalPos(action->getStartPos(), COORD{0, 0}));
	EXPECT_TRUE(equalPos(action->getEndPos(), COORD{ 15, 0 }));
}

TEST(UserActionHistoryTests, DontMergeWriteFromLeftSideActionsTest) {
	constexpr int nEntries = 3;
	UserActionHistory history{ std::chrono::milliseconds(10000000) };
	for (int i = nEntries - 1; i >= 0; i--) {
		auto action = makeWriteAction(
			{ static_cast<SHORT>(5 * i), 0 },
			{ static_cast<SHORT>(5 * (i + 1)), 0 },
			std::string{"test" + std::to_string(i)}
		);
		history.push(action);
	}
	EXPECT_EQ(history.getUndoActions().size(), nEntries);
	auto action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_EQ(action->getText(), "test0");
	EXPECT_TRUE(equalPos(action->getStartPos(), COORD{ 0, 0 }));
	EXPECT_TRUE(equalPos(action->getEndPos(), COORD{ 5, 0 }));
}

TEST(UserActionHistoryTests, MergeEraseFromLeftSideActionsTest) {
	constexpr int nEntries = 3;
	UserActionHistory history{ std::chrono::milliseconds(10000000) };
	for (int i = nEntries; i > 0; i--) {
		auto action = makeEraseAction(
			{ static_cast<SHORT>(5 * i), 0 },
			{ static_cast<SHORT>(5 * (i - 1)), 0 },
			std::string{"test" + std::to_string(i)}
		);
		history.push(action);
	}
	EXPECT_EQ(history.getUndoActions().size(), 1);
	auto action = history.undo().value_or(makeEmptyEraseAction());
	EXPECT_EQ(action->getText(), "test1test2test3");
	EXPECT_TRUE(equalPos(action->getStartPos(), COORD{ 15, 0 }));
	EXPECT_TRUE(equalPos(action->getEndPos(), COORD{ 0, 0 }));
}

TEST(UserActionHistoryTests, DontMergeEraseFromRightSideActionsTest) {
	constexpr int nEntries = 3;
	UserActionHistory history{ std::chrono::milliseconds(10000000) };
	for (int i = 1; i <= nEntries; i++) {
		auto action = makeEraseAction(
			{ static_cast<SHORT>(5 * i), 0 },
			{ static_cast<SHORT>(5 * (i - 1)), 0 },
			std::string{"test" + std::to_string(i)}
		);
		history.push(action);
	}
	EXPECT_EQ(history.getUndoActions().size(), 3);
	auto action = history.undo().value_or(makeEmptyEraseAction());
	EXPECT_EQ(action->getText(), "test3");
	EXPECT_TRUE(equalPos(action->getStartPos(), COORD{ 15, 0 }));
	EXPECT_TRUE(equalPos(action->getEndPos(), COORD{ 10, 0 }));
}

TEST(UserActionHistoryTests, SplitActionTest) {
	// ...[...(...)...]... //
	UserActionHistory history{ std::chrono::milliseconds(0) };
	auto action = makeWriteAction(COORD{ 0, 0 }, COORD{ 12, 0 }, "verylongtest");
	history.push(action);

	auto anotherUserAction = makeWriteAction(COORD{ 4, 0 }, COORD{ 8, 0 }, "test");
	COORD diffPos = COORD{ 4, 0 };
	history.affect(anotherUserAction, diffPos);

	EXPECT_EQ(history.getUndoActions().size(), 2);
	action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_TRUE(equalPos(action->getStartPos(), COORD{ 8, 0 }));
	EXPECT_TRUE(equalPos(action->getEndPos(), COORD{ 16, 0 }));
	EXPECT_EQ(action->getText(), "longtest");
	action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_TRUE(equalPos(action->getStartPos(), COORD{ 0, 0 }));
	EXPECT_TRUE(equalPos(action->getEndPos(), COORD{ 4, 0 }));
	EXPECT_EQ(action->getText(), "very");
}

TEST(UserActionHistoryTests, SplitActionWithEndlTest) {
	// ...[...(...)...]... //
	UserActionHistory history{ std::chrono::milliseconds(0) };
	auto action = makeWriteAction(COORD{ 0, 0 }, COORD{ 2, 1 }, "verylongtest\nxd");
	history.push(action);

	auto anotherUserAction = makeWriteAction(COORD{ 4, 0 }, COORD{ 0, 1 }, "ThisTextIsSoLong\n");
	COORD diffPos = COORD{ -4, 1 };
	history.affect(anotherUserAction, diffPos);

	EXPECT_EQ(history.getUndoActions().size(), 2);
	action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_TRUE(equalPos(action->getStartPos(), COORD{ 0, 1 }));
	EXPECT_TRUE(equalPos(action->getEndPos(), COORD{ 2, 2 }));
	EXPECT_EQ(action->getText(), "longtest\nxd");
	action = history.undo().value_or(makeEmptyWriteAction());
	EXPECT_TRUE(equalPos(action->getStartPos(), COORD{ 0, 0 }));
	EXPECT_TRUE(equalPos(action->getEndPos(), COORD{ 4, 0 }));
	EXPECT_EQ(action->getText(), "very");
}

//TEST(UserActionHistoryTests, CutActionFromRightTest) {
//	// ...[...(...]...)... //
//	UserActionHistory history{ std::chrono::milliseconds(0) };
//	auto action = makeWriteAction(COORD{ 0, 0 }, COORD{ 10, 0 }, "testing123");
//	history.push(action);
//
//	auto anotherUserAction = makeWriteAction(COORD{ 8, 0 }, COORD{ 12, 0 }, "testing123");
//	COORD diffPos = COORD{ 4, 0 };
//	history.affect(anotherUserAction, diffPos);
//
//	EXPECT_EQ(history.getUndoActions().size(), 1);
//	action = history.undo().value_or(makeEmptyWriteAction());
//	EXPECT_TRUE(equalPos(action->getStartPos(), COORD{ 0, 0 }));
//	EXPECT_TRUE(equalPos(action->getEndPos(), COORD{ 8, 0 }));
//}


//TEST(UserActionHistoryTests, CutActionFromLeftTest) {
//	// ...(...[...)...]... //
//	UserActionHistory history{ std::chrono::milliseconds(0) };
//	auto action = makeWriteAction(COORD{ 6, 0 }, COORD{ 16, 0 }, "testing123");
//	history.push(action);
//
//	auto anotherUserAction = makeWriteAction(COORD{ 0, 0 }, COORD{ 10, 0 }, "testing123");
//	COORD diffPos = COORD{ 10, 0 };
//	history.affect(anotherUserAction, diffPos);
//
//	EXPECT_EQ(history.getUndoActions().size(), 1);
//	action = history.undo().value_or(makeEmptyWriteAction());
//	EXPECT_TRUE(equalPos(action->getStartPos(), COORD{ 10, 0 }));
//	EXPECT_TRUE(equalPos(action->getEndPos(), COORD{ 16, 0 }));
//}

//TEST(UserActionHistoryTests, DeleteActionTest) {
//	// ...([......])... //
//	UserActionHistory history{ std::chrono::milliseconds(0) };
//	auto action = makeWriteAction(COORD{ 6, 0 }, COORD{ 16, 0 }, "testing123");
//	history.push(action);
//
//	auto anotherUserAction = makeWriteAction(COORD{ 6, 0 }, COORD{ 16, 0 }, "testing123");
//	COORD diffPos = COORD{ 10, 0 };
//	history.affect(anotherUserAction, diffPos);
//
//	EXPECT_EQ(history.getUndoActions().size(), 0);
//}