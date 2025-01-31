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


TEST(DocumentTests, JustTest) {
	// ...[...(...]...)... //
	Document doc{ "", 1, 0 };
	doc.write(0, "verylongline");
	doc.erase(0, 2);
	doc.setCursorPos(0, COORD{ 6, 0 });
	doc.erase(0, 2);
}

