#include "pch.h"
#include "server_document.h"
#include "client_document.h"
#include "pos_helpers.h"

bool testSegments(const TextContainer::Segments& actual, const TextContainer::Segments& expected) {
	EXPECT_EQ(actual.size(), expected.size());
	if (actual.size() != expected.size()) {
		return false;
	}
	for (int i = 0; i < actual.size(); i++) {
		EXPECT_EQ(actual[i], expected[i]);
	}
	return true;
}

bool coordsEq(COORD first, COORD second) {
	return first.X == second.X && first.Y == second.Y;
}

TEST(DocumentTests, DefaultConstructorTest) {
	ServerSiteDocument doc;
	EXPECT_TRUE(doc.get()[0].empty());
	EXPECT_TRUE(coordsEq(doc.getCursorPos(0), COORD{ 0, 0 }));
	EXPECT_TRUE(coordsEq(doc.getCursorPos(1), COORD{ -1, -1 }));
}

TEST(DocumentTests, TextConstructorTest) {
	const std::string txt = "Random text\ntest";
	ServerSiteDocument doc{ txt };
	EXPECT_TRUE(coordsEq(doc.getCursorPos(0), COORD{ 0, 0 }));
	EXPECT_TRUE(coordsEq(doc.getCursorPos(1), COORD{ -1, -1 }));
	EXPECT_EQ(doc.getText(), txt);
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocumentTests, TextConstructorLastLineEmptyTest) {
	const std::string txt = "Random text\n";
	ServerSiteDocument doc{ txt };
	EXPECT_TRUE(coordsEq(doc.getCursorPos(0), COORD{ 0, 0 }));
	EXPECT_TRUE(coordsEq(doc.getCursorPos(1), COORD{ -1, -1 }));
	EXPECT_EQ(doc.getText(), txt);
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocumentTests, SimpleWriteTest) {
	const std::string txt = "Random text\n";
	ServerSiteDocument doc;
	doc.write(0, txt);
	EXPECT_TRUE(coordsEq(doc.getCursorPos(0), COORD{ 0, 1 }));
	EXPECT_EQ(doc.getText(), txt);
	EXPECT_EQ(doc.get().size(), 2);
}

TEST(DocumentTests, SimpleEraseTest) {
	const std::string txt = "Random text\ntest";
	ServerSiteDocument doc{txt};
	doc.setCursorPos(0, std::move(COORD{ 4, 1 }));
	doc.erase(0, 5);
	EXPECT_TRUE(coordsEq(doc.getCursorPos(0), COORD{ 11, 0 }));
	EXPECT_EQ(doc.getText(), "Random text");
	EXPECT_EQ(doc.get().size(), 1);
}

TEST(DocumentTests, EraseUndoTest) {
	// ...[...(...]...)... //
	ServerSiteDocument doc{ "abcd", 1, 0 };
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
	ServerSiteDocument doc{ "", 1, 0 };
	doc.write(0, "test");
	doc.erase(0, 1);
	doc.undo(0);
	doc.undo(0);
	EXPECT_EQ(doc.getText(), "");
}

TEST(DocumentTests, WriteAndMoveEndlUndoRedoTest) {
	// ...[...(...]...)... //
	ServerSiteDocument doc{ "", 1, 0 };
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
	ServerSiteDocument doc{ "", 2, 0 };
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
	ServerSiteDocument doc{ "", 2, 0 };
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
	ServerSiteDocument doc{ "", 2, 0 };
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
	ServerSiteDocument doc{ "", 2, 0 };
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
	ServerSiteDocument doc{ "", 2, 0 };
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
	ServerSiteDocument doc{ "", 2, 0 };
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
	ServerSiteDocument doc{ "", 2, 0, options };

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
	ServerSiteDocument doc{ "", 1, 0, options };

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
	ServerSiteDocument doc{ "", 2, 0, options };

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
	ServerSiteDocument doc{ "some text", 1, 0, options };
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

TEST(DocumentTests, UndoMovedRedoTest) {
	history::HistoryManagerOptions options;
	options.mergeInterval = std::chrono::milliseconds{ 0 };
	ServerSiteDocument doc{ "", 3, 0, options };
	doc.write(0, "hello world");
	doc.write(1, " strawberry");
	doc.write(2, " kinematic");
	doc.undo(1);
	doc.setCursorPos(0, COORD{ 0, 0 });
	doc.write(0, "another text ");
	doc.redo(1);
	EXPECT_EQ(doc.getText(), "another text hello world strawberry kinematic");
}

TEST(DocumentTests, SegmentsAffectWriteFromLeftTest) {
	ClientSiteDocument doc{"some new text", 1, 0};
	doc.findSegments("new");
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), { {COORD(8, 0), COORD(11, 0)} });
}

TEST(DocumentTests, SegmentsAffectWriteFromLeftOnBorderTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 5, 0 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), { {COORD(8, 0), COORD(11, 0)} });
}

TEST(DocumentTests, SegmentsAffectWriteFromInsideSegmentTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 6, 0 });
	doc.write(0, "ADD");
	auto& segments = doc.getSegments();
	EXPECT_EQ(segments.size(), 0);
}

TEST(DocumentTests, SegmentsAffectWriteFromRightOnBorderTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 8, 0 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), { {COORD(5, 0), COORD(8, 0)} });
}

TEST(DocumentTests, SegmentsAffectWriteFromRightSiteTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 9, 0 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), {{COORD(5, 0), COORD(8, 0)} });
}

TEST(DocumentTests, MoreSegmentsAffectWriteFromLeftSiteTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 0, 0 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), {
		{COORD(8, 0), COORD(11, 0)},
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectWriteFromLeftOnBorderTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 5, 0 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), {
		{COORD(8, 0), COORD(11, 0)},
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectWriteFromCenterTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 6, 0 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), {
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectWriteFromRightOnBorderTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 8, 0 });
	doc.write(0, "ADD");
	auto& segments = doc.getSegments();
	testSegments(doc.getSegments(), {
		{COORD(5, 0), COORD(8, 0)},
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectWriteFromRightSiteTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 9, 0 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), {
		{COORD(5, 0), COORD(8, 0)},
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectWriteFromLeftSiteCentralSegmentTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 0, 1 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), {
		{COORD(5, 0), COORD(8, 0)},
		{COORD(8, 1), COORD(11, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectWriteFromLeftOnBorderCentralSegmentTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 5, 1 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), {
		{COORD(5, 0), COORD(8, 0)},
		{COORD(8, 1), COORD(11, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectWriteFromCenterCentralSegmentTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 6, 1 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), {
		{COORD(5, 0), COORD(8, 0)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectWriteFromRightOnBorderCentralSegmentTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 8, 1 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), {
		{COORD(5, 0), COORD(8, 0)},
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectWriteFromRightSiteCentralSegmentTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 9, 1 });
	doc.write(0, "ADD");
	testSegments(doc.getSegments(), { 
		{COORD(5, 0), COORD(8, 0)},
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, SegmentsAffectEraseFromLeftTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 3, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), { {COORD(2, 0), COORD(5, 0)} });
}

TEST(DocumentTests, SegmentsAffectEraseFromLeftOnBorderTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 5, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), { {COORD(2, 0), COORD(5, 0)} });
}

TEST(DocumentTests, SegmentsAffectEraseFromCenterLeftTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 6, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), {});
}

TEST(DocumentTests, SegmentsAffectEraseFromCenterTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 7, 0 });
	doc.erase(0, 1);
	testSegments(doc.getSegments(), {});
}

TEST(DocumentTests, SegmentsAffectEraseFromRightOnBorderTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 11, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), { {COORD{ 5 ,0 }, COORD{ 8, 0 }} });
}

TEST(DocumentTests, SegmentsAffectEraseFromRightSiteTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 13, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), { {COORD{ 5 ,0 }, COORD{ 8, 0 }} });
}

TEST(DocumentTests, MoreSegmentsAffectEraseFromLeftTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 3, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), { 
		{COORD(2, 0), COORD(5, 0)},
		{COORD(5, 1), COORD(8, 1)}, 
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectEraseFromLeftOnBorderTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 5, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), {
		{COORD(2, 0), COORD(5, 0)},
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectEraseFromCenterLeftTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 6, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), {
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectEraseFromCenterTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 7, 0 });
	doc.erase(0, 1);
	testSegments(doc.getSegments(), {
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectEraseFromCenterRightTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 9, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), {
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectEraseFromRightOnBorderTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 11, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), {
		{COORD(5, 0), COORD(8, 0)},
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectEraseFromRightSiteTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 13, 0 });
	doc.erase(0, 3);
	testSegments(doc.getSegments(), {
		{COORD(5, 0), COORD(8, 0)},
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)} });
}

TEST(DocumentTests, MoreSegmentsAffectHugeEraseTest) {
	ClientSiteDocument doc{ "some new text\nsome new text\nsome new text\nsome new text\nsome new text\nsome new text\nsome new text\nsome new text\nsome new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 3, 6 });
	doc.erase(0, 50);
	testSegments(doc.getSegments(), {
		{COORD(5, 0), COORD(8, 0)},
		{COORD(5, 1), COORD(8, 1)},
		{COORD(5, 2), COORD(8, 2)},
		{COORD(11, 2), COORD(14, 2)}, 
		{COORD(5, 3), COORD(8, 3)},
		{COORD(5, 4), COORD(8, 4)} });
}

TEST(DocumentTests, SegmentsAffectEraseBetweenTest) {
	ClientSiteDocument doc{ "some new text", 1, 0 };
	doc.findSegments("new");
	doc.setCursorPos(0, COORD{ 8, 0 });
	doc.setCursorAnchor(0, COORD{ 5, 0 });
	doc.erase(0, 1);
	testSegments(doc.getSegments(), {});
}