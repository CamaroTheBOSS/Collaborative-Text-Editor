#include "pch.h"
#include "new_text_container.h"

constexpr int clientWidth = 7;
using namespace v3;

TEST(NewTextContainerTest, AppendTestNoEndl) {
	NewTextContainer doc{ "AppendTest" };
	Cursor cursor{ Pos{ 10, 0 } };
	doc.write(cursor, "Appended text");

	auto expPos = Pos{ 23, 0 };
	auto expOff = 23;
	auto expLabel = ' ';
	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "AppendTestAppended text");
	EXPECT_EQ(cursor, Cursor(Pos{ 23, 0 }, 23, ' '));
}

TEST(NewTextContainerTest, AppendTestWithEndl) {
	NewTextContainer doc{ "AppendTest" };
	Cursor cursor{ Pos{ 10, 0 } };
	doc.write(cursor, "Appended text\n");

	EXPECT_EQ(doc.size(), 2);
	EXPECT_EQ(doc.getLine(0), "AppendTestAppended text");
	EXPECT_EQ(doc.getLine(1), "");
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 1 }, 0, ' '));
}

TEST(NewTextContainerTest, AppendTestOnlyEndl) {
	NewTextContainer doc{ "AppendTest" };
	Cursor cursor{ Pos{ 10, 0 } };
	doc.write(cursor, "\n");

	EXPECT_EQ(doc.size(), 2);
	EXPECT_EQ(doc.getLine(0), "AppendTest");
	EXPECT_EQ(doc.getLine(1), "");
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 1 }, 0, ' '));
}

TEST(NewTextContainerTest, AppendTestWithEndlAndText) {
	NewTextContainer doc{ "AppendTest" };
	Cursor cursor{ Pos{ 10, 0 } };
	doc.write(cursor, "Appended text\nMoreText");

	EXPECT_EQ(doc.size(), 2);
	EXPECT_EQ(doc.getLine(0), "AppendTestAppended text");
	EXPECT_EQ(doc.getLine(1), "MoreText");
	EXPECT_EQ(cursor, Cursor(Pos{ 8, 1 }, 8, ' '));
}

TEST(NewTextContainerTest, InsertTestNoEndl) {
	NewTextContainer doc{ "InsertTest" };
	Cursor cursor{ Pos{ 6, 0 } };
	doc.write(cursor, "Inserted text");

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "InsertInserted textTest");
	EXPECT_EQ(cursor, Cursor(Pos{ 19, 0 }, 19, 'T'));
}

TEST(NewTextContainerTest, InsertTestWithEndl) {
	NewTextContainer doc{ "InsertTest" };
	Cursor cursor{ Pos{ 6, 0 } };
	doc.write(cursor, "Inserted text\n");

	EXPECT_EQ(doc.size(), 2);
	EXPECT_EQ(doc.getLine(0), "InsertInserted text");
	EXPECT_EQ(doc.getLine(1), "Test");
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 1 }, 0, 'T'));
}

TEST(NewTextContainerTest, InsertTestWithEndlAndText) {
	NewTextContainer doc{ "InsertTest" };
	Cursor cursor{ Pos{ 6, 0 } };
	doc.write(cursor, "Inserted text\nMoreText");

	EXPECT_EQ(doc.size(), 2);
	EXPECT_EQ(doc.getLine(0), "InsertInserted text");
	EXPECT_EQ(doc.getLine(1), "MoreTextTest");
	EXPECT_EQ(cursor, Cursor(Pos{ 8, 1 }, 8, 'T'));
}

TEST(NewTextContainerTest, WriteJustEndl) {
	NewTextContainer doc{ "InsertTest" };
	Cursor cursor{ Pos{ 6, 0 } };
	doc.write(cursor, "\n");

	EXPECT_EQ(doc.size(), 2);
	EXPECT_EQ(doc.getLine(0), "Insert");
	EXPECT_EQ(doc.getLine(1), "Test");
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 1 }, 0, 'T'));
}

//TEST(NewTextContainerTest, WriteLetterAffectAnotherCursor) {
//	auto doc = prepTest("WriteTest\nMoreText\nLetsGO", Pos{ 3, 1 });
//	std::vector<Pos> startCursorPositions = { Pos{3, 0}, Pos{2, 1}, Pos{5, 1}, Pos{2, 2} };
//	std::vector<Pos> expectedCursorPositions = { Pos{ 12, 1 }, Pos{3, 0}, Pos{2, 1}, Pos{14, 1}, Pos{2, 2} };
//	std::vector<char> expectedLabels = { 'e', 't', 'r', 'e', 't' };
//	for (const auto& cursorPos : startCursorPositions) {
//		auto cursor = Cursor();
//		cursor.shiftTo(cursorPos, doc);
//		doc.addCursor(cursor);
//	}
//
//	auto& cursor = doc.getMyCursor();
//	doc.write(cursor, "addedText");
//
//	EXPECT_EQ(doc.getLine(0), "WriteTest");
//	EXPECT_EQ(doc.getLine(1), "MoraddedTexteText");
//	EXPECT_EQ(doc.getLine(2), "LetsGO");
//	EXPECT_EQ(doc.get().size(), 3);
//	for (int i = 0; i < expectedLabels.size(); i++) {
//		auto& docCursor = doc.getCursor(i);
//		EXPECT_EQ(docCursor.pos.X, expectedCursorPositions[i].X);
//		EXPECT_EQ(docCursor.pos.Y, expectedCursorPositions[i].Y);
//		EXPECT_EQ(docCursor.offset, expectedCursorPositions[i].X);
//		EXPECT_EQ(docCursor.label, expectedLabels[i]);
//	}
//}
//
//TEST(NewTextContainerTest, WriteNewLineAffectAnotherCursor) {
//	auto doc = prepTest("WriteTest\nMoreText\nLetsGO", Pos{ 3, 1 });
//	std::vector<Pos> startCursorPositions = { Pos{3, 0}, Pos{2, 1}, Pos{5, 1}, Pos{2, 2} };
//	std::vector<Pos> expectedCursorPositions = { Pos{ 0, 2 }, Pos{3, 0}, Pos{2, 1}, Pos{2, 2}, Pos{2, 3} };
//	std::vector<char> expectedLabels = { 'e', 't', 'r', 'e', 't' };
//	for (const auto& cursorPos : startCursorPositions) {
//		auto cursor = Cursor();
//		cursor.shiftTo(cursorPos, doc);
//		doc.addCursor(cursor);
//	}
//
//	auto& cursor = doc.getMyCursor();
//	doc.write(cursor, "\n");
//
//	EXPECT_EQ(doc.getLine(0), "WriteTest");
//	EXPECT_EQ(doc.getLine(1), "Mor");
//	EXPECT_EQ(doc.getLine(2), "eText");
//	EXPECT_EQ(doc.getLine(3), "LetsGO");
//	EXPECT_EQ(doc.get().size(), 4);
//	for (int i = 0; i < expectedLabels.size(); i++) {
//		auto& docCursor = doc.getCursor(i);
//		EXPECT_EQ(docCursor.pos.X, expectedCursorPositions[i].X);
//		EXPECT_EQ(docCursor.pos.Y, expectedCursorPositions[i].Y);
//		EXPECT_EQ(docCursor.offset, expectedCursorPositions[i].X);
//		EXPECT_EQ(docCursor.label, expectedLabels[i]);
//	}
//}
//
TEST(NewTextContainerTest, EraseLastOne) {
	NewTextContainer doc{ "EraseLastTest" };
	Cursor cursor{ Pos{ 13, 0 } };
	doc.erase(cursor, 1);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "EraseLastTes");
	EXPECT_EQ(cursor, Cursor(Pos{ 12, 0 }, 12, ' '));
}

TEST(NewTextContainerTest, EraseLastMultiple) {
	NewTextContainer doc{ "EraseLastTest" };
	Cursor cursor{ Pos{ 13, 0 } };
	doc.erase(cursor, 3);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "EraseLastT");
	EXPECT_EQ(cursor, Cursor(Pos{ 10, 0 }, 10, ' '));
}

TEST(NewTextContainerTest, EraseLastEndl) {
	NewTextContainer doc{ "EraseLastTest\n" };
	Cursor cursor{ Pos{ 0, 1 } };
	doc.erase(cursor, 1);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "EraseLastTest");
	EXPECT_EQ(cursor, Cursor(Pos{ 13, 0 }, 13, ' '));
}

TEST(NewTextContainerTest, EraseLastMultipleWithEndl) {
	NewTextContainer doc{ "EraseLastTest\nMoreText" };
	Cursor cursor{ Pos{ 8, 1 } };
	doc.erase(cursor, 10);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "EraseLastTes");
	EXPECT_EQ(cursor, Cursor(Pos{ 12, 0 }, 12, ' '));
}

TEST(NewTextContainerTest, EraseLastEmptyDoc) {
	NewTextContainer doc{ "" };
	Cursor cursor{ Pos{ 0, 0 } };
	doc.erase(cursor, 1);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "");
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 0 }, 0, ' '));
}

TEST(NewTextContainerTest, EraseMiddleOne) {
	NewTextContainer doc{ "EraseMiddleTest" };
	Cursor cursor{ Pos{ 6, 0 } };
	doc.erase(cursor, 1);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "EraseiddleTest");
	EXPECT_EQ(cursor, Cursor(Pos{ 5, 0 }, 5, 'i'));
}

TEST(NewTextContainerTest, EraseMiddleMultiple) {
	NewTextContainer doc{ "EraseMiddleTest" };
	Cursor cursor{ Pos{ 6, 0 } };
	doc.erase(cursor, 3);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "EraiddleTest");
	EXPECT_EQ(cursor, Cursor(Pos{ 3, 0 }, 3, 'i'));	
}

TEST(NewTextContainerTest, EraseMiddleWithEndl) {
	NewTextContainer doc{ "EraseMiddleTest\nMoreText" };
	Cursor cursor{ Pos{ 0, 1 } };
	doc.erase(cursor, 1);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "EraseMiddleTestMoreText");
	EXPECT_EQ(cursor, Cursor(Pos{ 15, 0 }, 15, 'M'));
}

TEST(NewTextContainerTest, EraseMiddleWithEndlAndText) {
	NewTextContainer doc{ "EraseMiddleTest\nMoreText" };
	Cursor cursor{ Pos{ 3, 1 } };
	doc.erase(cursor, 5);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "EraseMiddleTeseText");
	EXPECT_EQ(cursor, Cursor(Pos{ 14, 0 }, 14, 'e'));
}

TEST(NewTextContainerTest, EraseMiddleToDocStart) {
	NewTextContainer doc{ "EraseMiddleTest\nMoreText" };
	Cursor cursor{ Pos{ 3, 1 } };
	doc.erase(cursor, 40);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "eText");
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 0 }, 0, 'e'));
}

TEST(NewTextContainerTest, EraseToEmptyDoc) {
	NewTextContainer doc{ "EraseMiddleTest\nMoreText" };
	Cursor cursor{ Pos{ 8, 1 } };
	doc.erase(cursor, 40);

	EXPECT_EQ(doc.size(), 1);
	EXPECT_EQ(doc.getLine(0), "");
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 0 }, 0, ' '));
}

TEST(NewTextContainerTest, MoveLeftInTheMiddle) {
	NewTextContainer doc{ "MovementTest" };
	Cursor cursor{ Pos{ 3, 0 } };
	doc.moveLeft(cursor);
	EXPECT_EQ(cursor, Cursor(Pos{ 2, 0 }, 2, 'v'));
}

TEST(NewTextContainerTest, MoveLeftStartDoc) {
	NewTextContainer doc{ "MovementTest" };
	Cursor cursor{ Pos{ 0, 0 } };
	doc.moveLeft(cursor);
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 0 }, 0, 'M'));
}

TEST(NewTextContainerTest, MoveLeftToPrevLine) {
	NewTextContainer doc{ "MovementTest\nMoreText" };
	Cursor cursor{ Pos{ 0, 1 } };
	doc.moveLeft(cursor);
	EXPECT_EQ(cursor, Cursor(Pos{ 12, 0 }, 12, ' '));
}

TEST(NewTextContainerTest, MoveRightInTheMiddle) {
	NewTextContainer doc{ "MovementTest" };
	Cursor cursor{ Pos{ 3, 0 } };
	doc.moveRight(cursor);
	EXPECT_EQ(cursor, Cursor(Pos{ 4, 0 }, 4, 'm'));
}

TEST(NewTextContainerTest, MoveRightEndDoc) {
	NewTextContainer doc{ "MovementTest" };
	Cursor cursor{ Pos{ 12, 0 } };
	doc.moveRight(cursor);
	EXPECT_EQ(cursor, Cursor(Pos{ 12, 0 }, 12, ' '));
}

TEST(NewTextContainerTest, MoveRightToNextLine) {
	NewTextContainer doc{ "MovementTest\nMoreText" };
	Cursor cursor{ Pos{ 12, 0 } };
	doc.moveRight(cursor);
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 1 }, 0, 'M'));
}

TEST(NewTextContainerTest, MoveUpOnFirstLine) {
	NewTextContainer doc{ "MovementTest" };
	Cursor cursor{ Pos{ 5, 0 } };
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 0 }, 5, 'M'));
}

TEST(NewTextContainerTest, MoveUpToUpperLine) {
	NewTextContainer doc{ "Move\nTest" };
	Cursor cursor{ Pos{ 3, 1 } };
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 3, 0 }, 3, 'e'));
}

TEST(NewTextContainerTest, MoveUpWithinLine) {
	NewTextContainer doc{ "MovementTest" };
	Cursor cursor{ Pos{ 10, 0 } };
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 3, 0 }, 10, 'e'));
}

TEST(NewTextContainerTest, MoveUpFewLinesRememberOffset) {
	NewTextContainer doc{ "Move123\nText\nLongLin" };
	Cursor cursor{ Pos{ 5, 2 } };
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 4, 1 }, 5, ' '));
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 5, 0 }, 5, '2'));
}

TEST(NewTextContainerTest, MoveUpFewTimesWithinLine) {
	NewTextContainer doc{ "NewlineNewlineNewlineNewline" };
	Cursor cursor{ Pos{ 28, 0 } };
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 21, 0 }, 28, 'N'));
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 14, 0 }, 28, 'N'));
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 7, 0 }, 28, 'N'));
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 0, 0 }, 28, 'N'));
}

TEST(NewTextContainerTest, MoveUpFromShortToLongLine) {
	NewTextContainer doc{ "NewlineNewlineNewlineNewline\nShort" };
	Cursor cursor{ Pos{ 3, 1 } };
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 24, 0 }, 3, 'l'));
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 17, 0 }, 3, 'l'));
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 10, 0 }, 3, 'l'));
	doc.moveUp(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 3, 0 }, 3, 'l'));
}

TEST(NewTextContainerTest, MoveDownOnLastShortLine) {
	NewTextContainer doc{ "Move" };
	Cursor cursor{ Pos{ 2, 0 } };
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 4, 0 }, 2, ' '));
}

TEST(NewTextContainerTest, MoveDownToBottomLine) {
	NewTextContainer doc{ "Move\nTest" };
	Cursor cursor{ Pos{ 3, 0 } };
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 3, 1 }, 3, 't'));
}

TEST(NewTextContainerTest, MoveDownWithinLine) {
	NewTextContainer doc{ "MovementTest" };
	Cursor cursor{ Pos{ 3, 0 } };
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 10, 0 }, 3, 's'));
}

TEST(NewTextContainerTest, MoveDownFewLinesRememberOffset) {
	NewTextContainer doc{ "Move123\nText\nLongLin" };
	Cursor cursor{ Pos{ 5, 0 } };
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 4, 1 }, 5, ' '));
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 5, 2 }, 5, 'i'));
}

TEST(NewTextContainerTest, MoveDownFewTimesWithinLine) {
	NewTextContainer doc{ "NewlineNewlineNewlineNewline" };
	Cursor cursor{ Pos{ 0, 0 } };
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 7, 0 }, 0, 'N'));
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 14, 0 }, 0, 'N'));
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 21, 0 }, 0, 'N'));
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 28, 0 }, 0, ' '));
}

TEST(NewTextContainerTest, MoveDownFromShortToLongLine) {
	NewTextContainer doc{ "Short\nNewlineNewlineNewlineNewline" };
	Cursor cursor{ Pos{ 3, 0 } };
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 3, 1 }, 3, 'l'));
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 10, 1 }, 3, 'l'));
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 17, 1 }, 3, 'l'));
	doc.moveDown(cursor, clientWidth);
	EXPECT_EQ(cursor, Cursor(Pos{ 24, 1 }, 3, 'l'));
}
