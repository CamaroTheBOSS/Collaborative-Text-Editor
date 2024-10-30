#include "pch.h"

#include "renderer.h"



TEST(RendererTests, CursorSyncOneLinerDocTest) {
	Document doc{ "Oneliner test", 1, 0 };
	COORD dPos{ 5, 0 };
	doc.setCursorPos(0, dPos);

	ScrollableScreenBuffer screenBuffer{ SMALL_RECT(0, 0, 20, 20) };
	Cursor tCursor = screenBuffer.getMyTerminalCursor(doc);
	EXPECT_EQ(tCursor.pointedChar, 'n');
	EXPECT_EQ(tCursor.pos.X, 5);
	EXPECT_EQ(tCursor.pos.Y, 0);
}


TEST(RendererTests, CursorSyncOneEmptyLineTest) {
	Document doc{ "Oneliner test\n", 1, 0 };
	COORD dPos{ 0, 1 };
	doc.setCursorPos(0, dPos);

	ScrollableScreenBuffer screenBuffer{ SMALL_RECT(0, 0, 20, 20) };
	Cursor tCursor = screenBuffer.getMyTerminalCursor(doc);
	EXPECT_EQ(tCursor.pointedChar, ' ');
	EXPECT_EQ(tCursor.pos.X, 0);
	EXPECT_EQ(tCursor.pos.Y, 1);
}


TEST(RendererTests, CursorSyncOneLinerFilledTest) {
	Document doc{ "This is oneline test", 1, 0 };
	COORD dPos{ 20, 0 };
	doc.setCursorPos(0, dPos);

	ScrollableScreenBuffer screenBuffer{ SMALL_RECT(0, 0, 20, 20) };
	Cursor tCursor = screenBuffer.getMyTerminalCursor(doc);
	EXPECT_EQ(tCursor.pointedChar, ' ');
	EXPECT_EQ(tCursor.pos.X, 0);
	EXPECT_EQ(tCursor.pos.Y, 1);
	// doc's first line is filled with text, so cursor at the end should jump to the start of next line
}


TEST(RendererTests, CursorSyncOneLinerFilledWithEndlTest) {
	Document doc{ "This is oneline tes\n", 1, 0 };
	COORD dPos{ 20, 0 };
	doc.setCursorPos(0, dPos);

	ScrollableScreenBuffer screenBuffer{ SMALL_RECT(0, 0, 20, 20) };
	Cursor tCursor = screenBuffer.getMyTerminalCursor(doc);
	EXPECT_EQ(tCursor.pointedChar, ' ');
	EXPECT_EQ(tCursor.pos.X, 0);
	EXPECT_EQ(tCursor.pos.Y, 1);
	// doc's first line is filled with text and endl, so cursor at the end should jump to the start of next line
	// there was a bug with this case in the past
}


TEST(RendererTests, CursorSyncForLongerLinesTest) {
	Document doc{ "This is oneline but longer than screen width\n", 4, 0 };
	ScrollableScreenBuffer screenBuffer{ SMALL_RECT(0, 0, 20, 20) };
	
	std::vector<COORD> testDocPositions = { {5, 0}, {25, 0}, {45, 0}, {0, 1} };
	std::vector<COORD> desiredTerminalPositions = { {5, 0}, {5, 1}, {5, 2}, {0, 3} };
	std::vector<char> desiredPointedChars = { 'i', 'r', ' ', ' ' };
	for (int i = 0; i < testDocPositions.size(); i++) {
		doc.setCursorPos(i, testDocPositions[i]);
		Cursor tCursor = screenBuffer.getTerminalCursor(doc, i);
		EXPECT_EQ(tCursor.pointedChar, desiredPointedChars[i]);
		EXPECT_EQ(tCursor.pos.X, desiredTerminalPositions[i].X);
		EXPECT_EQ(tCursor.pos.Y, desiredTerminalPositions[i].Y);
	}
}