#include "pch.h"

#include "renderer.h"


ScreenBuffer getScreenBuffer() {
	ScreenBuffer screenBuffer;
	screenBuffer.srWindow = SMALL_RECT(0, 0, 20, 20);
	screenBuffer.dwSize = COORD{ 20, 20 };
	return screenBuffer;
}


TEST(RendererTests, CursorSyncOneLinerDocTest) {
	Document doc{ "Oneliner test", 1, 0 };
	COORD dPos{ 5, 0 };
	doc.setCursorPos(0, dPos);

	Renderer renderer;
	ScreenBuffer sb = getScreenBuffer();
	COORD tPos = renderer.synchronizeCursor(doc, dPos, sb);
	EXPECT_EQ(tPos.X, 5);
	EXPECT_EQ(tPos.Y, 0);
}


TEST(RendererTests, CursorSyncOneEmptyLineTest) {
	Document doc{ "Oneliner test\n", 1, 0 };
	COORD dPos{ 0, 1 };
	doc.setCursorPos(0, dPos);

	Renderer renderer;
	ScreenBuffer sb = getScreenBuffer();
	COORD tPos = renderer.synchronizeCursor(doc, dPos, sb);
	EXPECT_EQ(tPos.X, 0);
	EXPECT_EQ(tPos.Y, 1);
}


TEST(RendererTests, CursorSyncOneLinerFilledTest) {
	Document doc{ "This is oneline test", 1, 0 };
	COORD dPos{ 20, 0 };
	doc.setCursorPos(0, dPos);

	Renderer renderer;
	ScreenBuffer sb = getScreenBuffer();
	COORD tPos = renderer.synchronizeCursor(doc, dPos, sb);
	EXPECT_EQ(tPos.X, 0);
	EXPECT_EQ(tPos.Y, 1);
	// doc's first line is filled with text, so cursor at the end should jump to the start of next line
}


TEST(RendererTests, CursorSyncOneLinerFilledWithEndlTest) {
	Document doc{ "This is oneline tes\n", 1, 0 };
	COORD dPos{ 20, 0 };
	doc.setCursorPos(0, dPos);

	Renderer renderer;
	ScreenBuffer sb = getScreenBuffer();
	COORD tPos = renderer.synchronizeCursor(doc, dPos, sb);
	EXPECT_EQ(tPos.X, 0);
	EXPECT_EQ(tPos.Y, 1);
	// doc's first line is filled with text and endl, so cursor at the end should jump to the start of next line
	// there was a bug with this case in the past
}


TEST(RendererTests, CursorSyncForLongerLinesTest) {
	Document doc{ "This is oneline but longer than screen width\n", 4, 0 };
	ScreenBuffer sb = getScreenBuffer();
	Renderer renderer;

	std::vector<COORD> testDocPositions = { {5, 0}, {25, 0}, {45, 0}, {0, 1} };
	std::vector<COORD> desiredTerminalPositions = { {5, 0}, {5, 1}, {5, 2}, {0, 3} };
	for (int i = 0; i < testDocPositions.size(); i++) {
		doc.setCursorPos(i, testDocPositions[i]);
		COORD tPos = renderer.synchronizeCursor(doc, testDocPositions[i], sb);
		EXPECT_EQ(tPos.X, desiredTerminalPositions[i].X);
		EXPECT_EQ(tPos.Y, desiredTerminalPositions[i].Y);
	}
}