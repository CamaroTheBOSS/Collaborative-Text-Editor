#include "pch.h"
#include "text_container.h"

using Cursor = TextContainer::Cursor;

constexpr int clientWidth = 7;

template <typename... Args>
TextContainer prepTest(const std::string& startTxt, const COORD& startPos, const Args&&... args) {
	std::vector<Cursor> cursors = { Cursor() };
	TextContainer doc{ startTxt, cursors, 0 };
	auto& cursor = doc.getMyCursor();
	cursor.shiftTo(startPos, doc);
	return doc;
}

TEST(DocTests, AppendTestNoEndl) {
	auto doc = prepTest("AppendTest", COORD{ 10, 0 });
	auto& cursor = doc.getMyCursor();
	doc.write(cursor, "Appended text");

	EXPECT_EQ(doc.getLine(0), "AppendTestAppended text");
	EXPECT_EQ(cursor, Cursor(COORD{ 23, 0 }, 23, ' '));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, AppendTestWithEndl) {
	auto doc = prepTest("AppendTest", COORD{ 10, 0 });
	auto& cursor = doc.getMyCursor();
	doc.write(cursor, "Appended text\n");

	EXPECT_EQ(doc.getLine(0), "AppendTestAppended text");
	EXPECT_EQ(doc.getLine(1), "");
	EXPECT_EQ(cursor, Cursor(COORD{ 0, 1 }, 0, ' '));
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocTests, AppendTestWithEndlAndText) {
	auto doc = prepTest("AppendTest", COORD{ 10, 0 });
	auto& cursor = doc.getMyCursor();
	doc.write(cursor, "Appended text\nMoreText");

	EXPECT_EQ(doc.getLine(0), "AppendTestAppended text");
	EXPECT_EQ(doc.getLine(1), "MoreText");
	EXPECT_EQ(cursor, Cursor(COORD{ 8, 1 }, 8, ' '));
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocTests, InsertTestNoEndl) {
	auto doc = prepTest("InsertTest", COORD{ 6, 0 });
	auto& cursor = doc.getMyCursor();
	doc.write(cursor, "Inserted text");

	EXPECT_EQ(doc.getLine(0), "InsertInserted textTest");
	EXPECT_EQ(cursor, Cursor(COORD{ 19, 0 }, 19, 'T'));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, InsertTestWithEndl) {
	auto doc = prepTest("InsertTest", COORD{ 6, 0 });
	auto& cursor = doc.getMyCursor();
	doc.write(cursor, "Inserted text\n");

	EXPECT_EQ(doc.getLine(0), "InsertInserted text");
	EXPECT_EQ(doc.getLine(1), "Test");
	EXPECT_EQ(cursor, Cursor(COORD{ 0, 1 }, 0, 'T'));
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocTests, InsertTestWithEndlAndText) {
	auto doc = prepTest("InsertTest", COORD{ 6, 0 });
	auto& cursor = doc.getMyCursor();
	doc.write(cursor, "Inserted text\nMoreText");

	EXPECT_EQ(doc.getLine(0), "InsertInserted text");
	EXPECT_EQ(doc.getLine(1), "MoreTextTest");
	EXPECT_EQ(cursor, Cursor(COORD{ 8, 1 }, 8, 'T'));
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocTests, WriteJustEndl) {
	auto doc = prepTest("InsertTest", COORD{ 6, 0 });
	auto& cursor = doc.getMyCursor();
	doc.write(cursor, "\n");

	EXPECT_EQ(doc.getLine(0), "Insert");
	EXPECT_EQ(doc.getLine(1), "Test");
	EXPECT_EQ(cursor, Cursor(COORD{ 0, 1 }, 0, 'T'));
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocTests, WriteLetterAffectAnotherCursor) {
	auto doc = prepTest("WriteTest\nMoreText\nLetsGO", COORD{ 3, 1 });
	std::vector<COORD> startCursorPositions = { COORD{3, 0}, COORD{2, 1}, COORD{5, 1}, COORD{2, 2} };
	std::vector<COORD> expectedCursorPositions = { COORD{ 12, 1 }, COORD{3, 0}, COORD{2, 1}, COORD{14, 1}, COORD{2, 2} };
	std::vector<char> expectedLabels = { 'e', 't', 'r', 'e', 't' };
	for (const auto& cursorPos : startCursorPositions) {
		auto cursor = Cursor();
		cursor.shiftTo(cursorPos, doc);
		doc.addCursor(cursor);
	}

	auto& cursor = doc.getMyCursor();
	doc.write(cursor, "addedText");

	EXPECT_EQ(doc.getLine(0), "WriteTest");
	EXPECT_EQ(doc.getLine(1), "MoraddedTexteText");
	EXPECT_EQ(doc.getLine(2), "LetsGO");
	EXPECT_EQ(doc.get().size(), 3);
	for (int i = 0; i < expectedLabels.size(); i++) {
		auto& docCursor = doc.getCursor(i);
		EXPECT_EQ(docCursor.pos.X, expectedCursorPositions[i].X);
		EXPECT_EQ(docCursor.pos.Y, expectedCursorPositions[i].Y);
		EXPECT_EQ(docCursor.offset, expectedCursorPositions[i].X);
		EXPECT_EQ(docCursor.label, expectedLabels[i]);
	}
}

TEST(DocTests, WriteNewLineAffectAnotherCursor) {
	auto doc = prepTest("WriteTest\nMoreText\nLetsGO", COORD{ 3, 1 });
	std::vector<COORD> startCursorPositions = { COORD{3, 0}, COORD{2, 1}, COORD{5, 1}, COORD{2, 2} };
	std::vector<COORD> expectedCursorPositions = { COORD{ 0, 2 }, COORD{3, 0}, COORD{2, 1}, COORD{2, 2}, COORD{2, 3} };
	std::vector<char> expectedLabels = { 'e', 't', 'r', 'e', 't' };
	for (const auto& cursorPos : startCursorPositions) {
		auto cursor = Cursor();
		cursor.shiftTo(cursorPos, doc);
		doc.addCursor(cursor);
	}
	
	auto& cursor = doc.getMyCursor();
	doc.write(cursor, "\n");

	EXPECT_EQ(doc.getLine(0), "WriteTest");
	EXPECT_EQ(doc.getLine(1), "Mor");
	EXPECT_EQ(doc.getLine(2), "eText");
	EXPECT_EQ(doc.getLine(3), "LetsGO");
	EXPECT_EQ(doc.get().size(), 4);
	for (int i = 0; i < expectedLabels.size(); i++) {
		auto& docCursor = doc.getCursor(i);
		EXPECT_EQ(docCursor.pos.X, expectedCursorPositions[i].X);
		EXPECT_EQ(docCursor.pos.Y, expectedCursorPositions[i].Y);
		EXPECT_EQ(docCursor.offset, expectedCursorPositions[i].X);
		EXPECT_EQ(docCursor.label, expectedLabels[i]);
	}
}

TEST(DocTests, EraseLastOne) {
	auto doc = prepTest("EraseLastTest", COORD{ 13, 0 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 1);

	EXPECT_EQ(doc.getLine(0), "EraseLastTes");
	EXPECT_EQ(cursor, Cursor(COORD{ 12, 0 }, 12, ' '));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, EraseLastMultiple) {
	auto doc = prepTest("EraseLastTest", COORD{ 13, 0 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 3);

	EXPECT_EQ(doc.getLine(0), "EraseLastT");
	EXPECT_EQ(cursor, Cursor(COORD{ 10, 0 }, 10, ' '));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, EraseLastEndl) {
	auto doc = prepTest("EraseLastTest\n", COORD{ 0, 1 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 1);

	EXPECT_EQ(doc.getLine(0), "EraseLastTest");
	EXPECT_EQ(cursor, Cursor(COORD{ 13, 0 }, 13, ' '));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, EraseLastMultipleWithEndl) {
	auto doc = prepTest("EraseLastTest\nMoreText", COORD{ 8, 1 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 10);

	EXPECT_EQ(doc.getLine(0), "EraseLastTes");
	EXPECT_EQ(cursor, Cursor(COORD{ 12, 0 }, 12, ' '));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, EraseLastEmptyDoc) {
	auto doc = prepTest("", COORD{ 0, 0 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 1);

	EXPECT_EQ(doc.getLine(0), "");
	EXPECT_EQ(cursor, Cursor(COORD{ 0, 0 }, 0, ' '));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, EraseMiddleOne) {
	auto doc = prepTest("EraseMiddleTest", COORD{ 6, 0 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 1);

	EXPECT_EQ(doc.getLine(0), "EraseiddleTest");
	EXPECT_EQ(cursor, Cursor(COORD{ 5, 0 }, 5, 'i'));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, EraseMiddleMultiple) {
	auto doc = prepTest("EraseMiddleTest", COORD{ 6, 0 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 3);

	EXPECT_EQ(doc.getLine(0), "EraiddleTest");
	EXPECT_EQ(cursor, Cursor(COORD{ 3, 0 }, 3, 'i'));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, EraseMiddleWithEndl) {
	auto doc = prepTest("EraseMiddleTest\nMoreText", COORD{ 0, 1 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 1);

	EXPECT_EQ(doc.getLine(0), "EraseMiddleTestMoreText");
	EXPECT_EQ(cursor, Cursor(COORD{ 15, 0 }, 15, 'M'));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, EraseMiddleWithEndlAndText) {
	auto doc = prepTest("EraseMiddleTest\nMoreText", COORD{ 3, 1 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 5);

	EXPECT_EQ(doc.getLine(0), "EraseMiddleTeseText");
	EXPECT_EQ(cursor, Cursor(COORD{ 14, 0 }, 14, 'e'));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, EraseMiddleToDocStart) {
	auto doc = prepTest("EraseMiddleTest\nMoreText", COORD{ 3, 1 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 40);

	EXPECT_EQ(doc.getLine(0), "eText");
	EXPECT_EQ(cursor, Cursor(COORD{ 0, 0 }, 0, 'e'));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, EraseToEmptyDoc) {
	auto doc = prepTest("EraseMiddleTest\nMoreText", COORD{ 8, 1 });
	auto& cursor = doc.getMyCursor();
	doc.erase(cursor, 40);

	EXPECT_EQ(doc.getLine(0), "");
	EXPECT_EQ(cursor, Cursor(COORD{ 0, 0 }, 0, ' '));
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocTests, MoveLeftInTheMiddle) {
	auto doc = prepTest("MovementTest", COORD{ 3, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveLeft(doc);
	EXPECT_EQ(cursor, Cursor(COORD{ 2, 0 }, 2, 'v'));
}

TEST(DocTests, MoveLeftStartDoc) {
	auto doc = prepTest("MovementTest", COORD{ 0, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveLeft(doc);
	EXPECT_EQ(cursor, Cursor(COORD{ 0, 0 }, 0, 'M'));
}

TEST(DocTests, MoveLeftToPrevLine) {
	auto doc = prepTest("MovementTest\nMoreText", COORD{ 0, 1 });
	auto& cursor = doc.getMyCursor();
	cursor.moveLeft(doc);
	EXPECT_EQ(cursor, Cursor(COORD{ 12, 0 }, 12, ' '));
}

TEST(DocTests, MoveRightInTheMiddle) {
	auto doc = prepTest("MovementTest", COORD{ 3, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveRight(doc);
	EXPECT_EQ(cursor, Cursor(COORD{ 4, 0 }, 4, 'm'));
}

TEST(DocTests, MoveRightEndDoc) {
	auto doc = prepTest("MovementTest", COORD{ 12, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveRight(doc);
	EXPECT_EQ(cursor, Cursor(COORD{ 12, 0 }, 12, ' '));
}

TEST(DocTests, MoveRightToNextLine) {
	auto doc = prepTest("MovementTest\nMoreText", COORD{ 12, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveRight(doc);
	EXPECT_EQ(cursor, Cursor(COORD{ 0, 1 }, 0, 'M'));
}

TEST(DocTests, MoveUpOnFirstLine) {
	auto doc = prepTest("MovementTest", COORD{ 5, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 5, 0 }, 5, 'e'));
}

TEST(DocTests, MoveUpToUpperLine) {
	auto doc = prepTest("Move\nTest", COORD{ 3, 1 });
	auto& cursor = doc.getMyCursor();
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 3, 0 }, 3, 'e'));
}

TEST(DocTests, MoveUpWithinLine) {
	auto doc = prepTest("MovementTest", COORD{ 10, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 3, 0 }, 3, 'e'));
}

TEST(DocTests, MoveUpFewLinesRememberOffset) {
	auto doc = prepTest("Move123\nText\nLongLin", COORD{ 5, 2 });
	auto& cursor = doc.getMyCursor();
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 4, 1 }, 5, ' '));
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 5, 0 }, 5, '2'));
}

TEST(DocTests, MoveUpFewTimesWithinLine) {
	auto doc = prepTest("NewlineNewlineNewlineNewline", COORD{ 28, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 21, 0 }, 0, 'N'));
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 14, 0 }, 0, 'N'));
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 7, 0 }, 0, 'N'));
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 0, 0 }, 0, 'N'));
}

TEST(DocTests, MoveUpFromShortToLongLine) {
	auto doc = prepTest("NewlineNewlineNewlineNewline\nShort", COORD{ 3, 1 });
	auto& cursor = doc.getMyCursor();
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 24, 0 }, 3, 'l'));
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 17, 0 }, 3, 'l'));
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 10, 0 }, 3, 'l'));
	cursor.moveUp(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 3, 0 }, 3, 'l'));
}

TEST(DocTests, MoveDownOnLastShortLine) {
	auto doc = prepTest("Move", COORD{ 2, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 2, 0 }, 2, 'v'));
}

TEST(DocTests, MoveDownToBottomLine) {
	auto doc = prepTest("Move\nTest", COORD{ 3, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 3, 1 }, 3, 't'));
}

TEST(DocTests, MoveDownWithinLine) {
	auto doc = prepTest("MovementTest", COORD{ 3, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 10, 0 }, 3, 's'));
}

TEST(DocTests, MoveDownFewLinesRememberOffset) {
	auto doc = prepTest("Move123\nText\nLongLin", COORD{ 5, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 4, 1 }, 5, ' '));
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 5, 2 }, 5, 'i'));
}

TEST(DocTests, MoveDownFewTimesWithinLine) {
	auto doc = prepTest("NewlineNewlineNewlineNewline", COORD{ 0, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 7, 0 }, 0, 'N'));
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 14, 0 }, 0, 'N'));
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 21, 0 }, 0, 'N'));
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 28, 0 }, 0, ' '));
}

TEST(DocTests, MoveDownFromShortToLongLine) {
	auto doc = prepTest("Short\nNewlineNewlineNewlineNewline", COORD{ 3, 0 });
	auto& cursor = doc.getMyCursor();
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 3, 1 }, 3, 'l'));
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 10, 1 }, 3, 'l'));
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 17, 1 }, 3, 'l'));
	cursor.moveDown(doc, clientWidth);
	EXPECT_EQ(cursor, Cursor(COORD{ 24, 1 }, 3, 'l'));
}
