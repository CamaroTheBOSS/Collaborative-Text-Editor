#include "pch.h"
#include "document.h"

//#include <random>
//#include <array>

//auto getRandomEngine() {
//	std::random_device device;
//	std::random_device::result_type data[(std::mt19937::state_size - 1) / sizeof(device()) + 1];
//	std::generate(std::begin(data), std::end(data), std::ref(device));
//	std::seed_seq seed{std::begin(data), std::end(data)};
//	return std::mt19937(seed);
//}
//
//static auto randomEngine = getRandomEngine();
//static constexpr std::array<const char*, 5> randomText{
//		"letsgo",
//		"twolines\nmakarena",
//		"\n",
//		"111\n\n222\n",
//		"ousheet\n"
//};
//
//int makeRandomAction(Document& doc) {
//	std::uniform_int_distribution<> actionDist(0, 200);
//	std::uniform_int_distribution<> textDist(0, 5);
//	std::uniform_int_distribution<> eraseDist(1, 15);
//
//	int action = actionDist(randomEngine);
//	int key = 0;
//	if (action < 120) {
//		std::string seq{randomText[textDist(randomEngine)]};
//		doc.write(0, seq);
//	}
//	else if (action < 160) {
//		int eraseSize = eraseDist(randomEngine);
//		doc.erase(0, eraseSize);
//	}
//	else if (action < 170) {
//		doc.moveCursorLeft(0, 0);
//	}
//	else if (action < 180) {
//		doc.moveCursorRight(0, 0);
//	}
//	else if (action < 190) {
//		doc.moveCursorDown(0, 20, 0);
//	}
//	else {
//		doc.moveCursorUp(0, 20, 0);
//	}
//	return key;
//}


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

//TEST(DocumentTests, RandomUndoRedoTest) {
//	Document doc;
//	int nActions = 20;
//	for (int i = 0; i < nActions; i++) {
//		makeRandomAction(doc);
//	}
//	std::vector<UndoReturn> undoReturns;
//	for (int i = 0; i < nActions; i++) {
//		undoReturns.emplace_back(doc.undo(0));
//	}
//
//}