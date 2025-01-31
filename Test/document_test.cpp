#include "pch.h"
#include "document.h"


bool coordsEq(COORD first, COORD second) {
	return first.X == second.X && first.Y == second.Y;
}

TEST(DocumentTests, DefaultConstructorTest) {
	Document doc;
	EXPECT_TRUE(doc.get()[0].empty());
	EXPECT_TRUE(coordsEq(doc.getCursorPos(0), COORD{ 0, 0 }));
	EXPECT_TRUE(coordsEq(doc.getCursorPos(1), COORD{ -1, -1 }));
}

TEST(DocumentTests, TextConstructorTest) {
	const std::string txt = "Random text\ntest";
	Document doc{ txt };
	EXPECT_TRUE(coordsEq(doc.getCursorPos(0), COORD{ 0, 0 }));
	EXPECT_TRUE(coordsEq(doc.getCursorPos(1), COORD{ -1, -1 }));
	EXPECT_EQ(doc.getText(), txt);
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocumentTests, TextConstructorLastLineEmptyTest) {
	const std::string txt = "Random text\n";
	Document doc{ txt };
	EXPECT_TRUE(coordsEq(doc.getCursorPos(0), COORD{ 0, 0 }));
	EXPECT_TRUE(coordsEq(doc.getCursorPos(1), COORD{ -1, -1 }));
	EXPECT_EQ(doc.getText(), txt);
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocumentTests, SimpleWriteTest) {
	const std::string txt = "Random text\n";
	Document doc;
	doc.write(0, txt);
	EXPECT_TRUE(coordsEq(doc.getCursorPos(0), COORD{ 0, 1 }));
	EXPECT_EQ(doc.getText(), txt);
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocumentTests, SimpleEraseTest) {
	const std::string txt = "Random text\ntest";
	Document doc{txt};
	doc.setCursorPos(0, std::move(COORD{ 4, 1 }));
	doc.erase(0, 5);
	EXPECT_TRUE(coordsEq(doc.getCursorPos(0), COORD{ 11, 0 }));
	EXPECT_EQ(doc.getText(), "Random text");
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocumentTests, EraseUndoTest) {
	// ...[...(...]...)... //
	Document doc{ "abcd", 1, 0 };
	doc.setCursorPos(0, doc.getEndPos());
	for (int i = 0; i < 4; i++) {
		doc.erase(0, 1);
	}
	for (int i = 0; i < 4; i++) {
		doc.undo(0);
	}
	EXPECT_EQ(doc.getText(), "abcd");
}

TEST(DocumentTests, WriteAndEraseUndoTest) {
	// ...[...(...]...)... //
	Document doc{ "", 1, 0 };
	doc.write(0, "test");
	doc.erase(0, 1);
	doc.undo(0);
	doc.undo(0);
	EXPECT_EQ(doc.getText(), "");
}

TEST(DocumentTests, WriteAndMoveEndlUndoRedoTest) {
	// ...[...(...]...)... //
	Document doc{ "", 1, 0 };
	doc.write(0, "lower");
	doc.setCursorPos(0, COORD{ 0, 0 });
	doc.write(0, "\n");
	doc.setCursorPos(0, COORD{ 0, 0 });
	doc.write(0, "upper");
	for (int i = 0; i < 3; i++) {
		doc.undo(0);
	}
	for (int i = 0; i < 3; i++) {
		doc.redo(0);
	}
	EXPECT_EQ(doc.getText(), "upper\nlower");
}

TEST(DocumentTests, TwoUserHistoryUndoTest) {
	// ...[...(...]...)... //
	Document doc{ "", 2, 0 };
	doc.write(0, "first user text");
	doc.setCursorPos(1, COORD{6, 0});
	doc.write(1, "BREAK");

	doc.undo(0);
	doc.undo(0);
	EXPECT_EQ(doc.getText(), "BREAK");
	doc.redo(0);
	doc.redo(0);
	EXPECT_EQ(doc.getText(), "first BREAKuser text");
}

TEST(DocumentTests, TwoUserHistoryUndoWithEndlTest) {
	// ...[...(...]...)... //
	Document doc{ "", 2, 0 };
	doc.write(0, "first user text");
	doc.setCursorPos(0, COORD{ 0, 0 });
	doc.write(0, "\n");
	doc.setCursorPos(1, COORD{ 6, 1 });
	doc.write(1, "BREAK");

	for (int i = 0; i < 3; i++) {
		doc.undo(0);
	}
	EXPECT_EQ(doc.getText(), "BREAK");
	for (int i = 0; i < 3; i++) {
		doc.redo(0);
	}
	EXPECT_EQ(doc.getText(), "\nfirst BREAKuser text");
}

TEST(DocumentTests, TwoUserHistoryUndoWithErasesTest) {
	// ...[...(...]...)... //
	Document doc{ "", 2, 0 };
	doc.write(0, "first line\n");
	doc.write(0, "second line");
	doc.setCursorPos(0, COORD{ 0, 0 });
	doc.write(0, "\n");
	doc.setCursorPos(0, COORD{ 0, 0 });
	doc.write(0, "\n");
	doc.setCursorPos(0, COORD{ 0, 0 });
	doc.write(0, "include");

	doc.setCursorPos(1, COORD{ 6, 3 });
	doc.write(1, "BREAK");

	for (int i = 0; i < 5; i++) {
		doc.undo(0);
	}
	EXPECT_EQ(doc.getText(), "BREAK");
	for (int i = 0; i < 5; i++) {
		doc.redo(0);
	}
	EXPECT_EQ(doc.getText(), "include\n\nfirst line\nsecondBREAK line");
}

TEST(DocumentTests, TwoUserWriteWithEraseBeforeInterruptionUndoTest) {
	// ...[...(...]...)... //
	Document doc{ "", 2, 0 };
	doc.write(0, "first");
	doc.erase(0, 2);
	doc.write(0, "st user");
	doc.setCursorPos(1, COORD{ 5, 0 });
	doc.write(1, "BREAK");
	for (int i = 0; i < 4; i++) {
		doc.undo(0);
	}
	EXPECT_EQ(doc.getText(), "BREAK");
	for (int i = 0; i < 4; i++) {
		doc.redo(0);
	}
	EXPECT_EQ(doc.getText(), "firstBREAK user");
}

TEST(DocumentTests, TwoUserWriteWithEraseAfterInterruptionUndoTest) {
	// ...[...(...]...)... //
	Document doc{ "", 2, 0 };
	doc.write(0, "firxd");
	doc.erase(0, 2);
	doc.write(0, "st user");
	doc.setCursorPos(1, COORD{ 3, 0 });
	doc.write(1, "BREAK");
	for (int i = 0; i < 4; i++) {
		doc.undo(0);
	}
	EXPECT_EQ(doc.getText(), "BREAK");
	for (int i = 0; i < 4; i++) {
		doc.redo(0);
	}
	EXPECT_EQ(doc.getText(), "firBREAKst user");
}

TEST(DocumentTests, StdMovedWriteWithRelationshipEraseUndoTest) {
	// ...[...(...]...)... //
	Document doc{ "", 2, 0 };
	doc.write(0, "verylongline");
	doc.setCursorPos(1, COORD{ 4, 0 });
	doc.erase(1, 2);
	doc.undo(0);
	doc.undo(1);
	EXPECT_EQ(doc.getText(), "ry");
}

TEST(DocumentTests, BufferOverloadedWriteWithRelationshipDeletedEraseUndoTest) {
	// This is connected to crash. It happened when WriteAction had some Erases connected with itself
	// in eraseRegistry and then it was destroyed e.g. by ActionHistory which is dropping the eldest
	// actions when ActionHistory reach its capacity
	history::HistoryManagerOptions options;
	options.capacity = 3;
	options.mergeInterval = std::chrono::milliseconds{ 0 };
	Document doc{ "", 2, 0, options };

	doc.write(0, "verylongline");
	doc.setCursorPos(1, COORD{ 4, 0 });
	doc.erase(1, 2);
	doc.erase(0, 4);
	doc.write(0, "\ntext");
	doc.erase(0, 2);
	doc.undo(1);
	EXPECT_EQ(doc.getText(), "verylong\nte");
}

TEST(DocumentTests, ObsUndoTest) {
	history::HistoryManagerOptions options;
	options.mergeInterval = std::chrono::milliseconds{ 0 };
	Document doc{ "", 1, 0, options };

	doc.write(0, "first");
	doc.setCursorPos(0, COORD{ 2, 0 });
	doc.erase(0, 1);
	doc.write(0, "BREAK");
	doc.setCursorPos(0, COORD{ 8, 0 });
	doc.erase(0, 4);
	doc.write(0, "123");
	EXPECT_EQ(doc.getText(), "fBRE123t");
	for (int i = 0; i < 6; i++) {
		doc.undo(0);
	}
	EXPECT_EQ(doc.getText(), "");
}

TEST(DocumentTests, DoubleEraseInWriteUndoTest) {
	// Not implemented -> requires tracking all writes/erases history in each WriteAction to make correct
	// Needs to call affect on all related Erase actions after undo to positionate better history on WriteAction
	history::HistoryManagerOptions options;
	options.mergeInterval = std::chrono::milliseconds{ 0 };
	Document doc{ "", 2, 0, options };

	doc.write(0, "verylongline");
	doc.setCursorPos(0, COORD{ 10, 0 });
	doc.erase(0, 2);
	doc.setCursorPos(1, COORD{ 4, 0 });
	doc.erase(1, 2);
	EXPECT_EQ(doc.getText(), "velongne");
	for (int i = 0; i < 2; i++) {
		doc.undo(0);
	}
	EXPECT_EQ(doc.getText(), "");
}

TEST(DocumentTests, MergeEraseUndoTest) {
	// Merging erases doesn't work. This is caused that when action is created, it firstly do affect logic
	// and then is trying to merge. Because candidate for erase always will affect and move another erase
	// there is no chance for merge to happen in practice. A solution for that problem is try merging before
	// affect, but it creates some problems with affect then, because in such situation in affect we need
	// to use whole merged action which is already in UndoActions, so affect should omit itself
	history::HistoryManagerOptions options;
	options.mergeInterval = std::chrono::milliseconds{ 100000 };
	Document doc{ "some text", 1, 0, options };
	doc.setCursorPos(0, COORD{ 9, 0 });
	doc.erase(0, 1);
	doc.erase(0, 1);
	doc.erase(0, 1);
	doc.erase(0, 1);
	EXPECT_EQ(doc.getText(), "some ");
	for (int i = 0; i < 1; i++) {
		doc.undo(0);
	}
	EXPECT_EQ(doc.getText(), "some text");
}