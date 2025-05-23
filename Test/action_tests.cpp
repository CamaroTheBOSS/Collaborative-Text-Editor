#include "pch.h"
#include "action_write.h"
#include "action_erase.h"
#include "pos_helpers.h"
#include "text_container.h"

using ActionPtr = Action::ActionPtr;

std::vector<std::string> getSplitText() {
	return { "firstline", "secondline", "thirdline" };
}

std::vector<std::string> getOtherSplitText() {
	return { "microphones", "headphones", "mouses" };
}

std::vector<std::string> getOtherShortSplitText() {
	return { "microphones" };
}

TEST(ContainerTests, HappySplitTextTest) {
	std::vector<std::string> text = getSplitText();
	TextContainer container{ text };
	std::vector<std::string> expected{ "ondline", "thirdline" };
	auto splittedText = container.split(COORD{ 3, 1 });
	EXPECT_EQ(splittedText.get(), expected);
	EXPECT_EQ(container.getText(), "firstline\nsec");
}

TEST(ContainerTests, WrongSplitPointSplitTextTest) {
	std::vector<std::string> text = getSplitText();
	TextContainer container{ text };
	std::vector<std::string> expected{""};
	auto splittedText = container.split(COORD{ -1, 1 });
	EXPECT_EQ(splittedText.get(), expected);
	splittedText = container.split(COORD{ -1, -1 });
	EXPECT_EQ(splittedText.get(), expected);
	splittedText = container.split(COORD{ 0, 4 });
	EXPECT_EQ(splittedText.get(), expected);
	EXPECT_EQ(container.getText(), "firstline\nsecondline\nthirdline");
}

TEST(ContainerTests, FirstLineSplitTextTest) {
	std::vector<std::string> text = getSplitText();
	TextContainer container{ text };
	std::vector<std::string> expected{ "stline", "secondline", "thirdline" };
	auto splittedText = container.split(COORD{ 3, 0 });
	EXPECT_EQ(splittedText.get(), expected);
	EXPECT_EQ(container.getText(), "fir");
}

TEST(ContainerTests, LastLineSplitTextTest) {
	std::vector<std::string> text = getSplitText();
	TextContainer container{ text };
	std::vector<std::string> expected{ "rdline" };
	auto splittedText = container.split(COORD{ 3, 2 });
	EXPECT_EQ(splittedText.get(), expected);
	EXPECT_EQ(container.getText(), "firstline\nsecondline\nthi");
}

TEST(ContainerTests, FullSplitTextTest) {
	std::vector<std::string> text = getSplitText();
	TextContainer container{ text };
	std::vector<std::string> expected{ "firstline", "secondline", "thirdline" };

	auto splittedText = container.split(COORD{ 0, 0 });
	EXPECT_EQ(splittedText.get(), expected);
	EXPECT_EQ(container.getText(), "");
}

TEST(ContainerTests, NoSplitTextTest) {
	std::vector<std::string> text = getSplitText();
	TextContainer container{ text };
	std::vector<std::string> expected{""};

	auto splittedText = container.split(COORD{ 9, 2 });
	EXPECT_EQ(splittedText.get(), expected);
	EXPECT_EQ(container.getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, LastLetterSplitTextTest) {
	std::vector<std::string> text = getSplitText();
	TextContainer container{ text };
	std::vector<std::string> expected{"e"};

	auto splittedText = container.split(COORD{ 8, 2 });
	EXPECT_EQ(splittedText.get(), expected);
	EXPECT_EQ(container.getText(), "firstline\nsecondline\nthirdlin");
}

TEST(ActionTests, WriteWriteAffectFromLeftSideTest) {
	COORD startPos = COORD{ 5, 0 };
	COORD otherStartPos = COORD{ 4, 0 }; 
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(7, 2));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, WriteWriteAffectFromLeftStartTest) {
	COORD startPos = COORD{ 5, 0 };
	COORD otherStartPos = COORD{ 5, 0 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(6, 2));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, WriteWriteAffectFromRightStartTest) {
	COORD startPos = COORD{ 5, 0 };
	COORD otherStartPos = COORD{ 9, 2 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(5, 0));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, WriteWriteAffectFromRightSideTest) {
	COORD startPos = COORD{ 5, 0 };
	COORD otherStartPos = COORD{ 11, 2 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(5, 0));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, WriteWriteAffectFromCenterTest) {
	COORD startPos = COORD{ 5, 0 };
	COORD otherStartPos = COORD{ 4, 1 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	auto newAction = otherAction->affect(*action).first.value();

	EXPECT_EQ(action->getLeftPos(), COORD(5, 0));
	EXPECT_EQ(action->getText(), "firstline\nseco");
	EXPECT_EQ(newAction->getLeftPos(), COORD(6, 3));
	EXPECT_EQ(newAction->getText(), "ndline\nthirdline");
}

TEST(ActionTests, WriteWriteBelowOneLineTest) {
	COORD startPos = COORD{ 5, 4 };
	COORD otherStartPos = COORD{ 11, 0 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(5, 6));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, WriteWriteBelowOneLineOnlyOneLineTest) {
	COORD startPos = COORD{ 5, 4 };
	COORD otherStartPos = COORD{ 11, 0 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherShortSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(5, 4));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}


TEST(ActionTests, WriteEraseAffectFromLeftSideTest) {
	COORD startPos = COORD{ 10, 4 };
	COORD otherStartPos = COORD{ 6, 4 };
	COORD otherEndPos = COORD{ 4, 2 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(8, 2));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, WriteEraseAffectFromLeftSideInOneLineTest) {
	COORD startPos = COORD{ 14, 4 };
	COORD otherStartPos = COORD{ 13, 4 };
	COORD otherEndPos = COORD{ 2, 4 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherShortSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(3, 4));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, WriteEraseCutFromLeftSideTest) {
	COORD startPos = COORD{ 10, 4 };
	COORD otherStartPos = COORD{ 6, 5 };
	COORD otherEndPos = COORD{ 4, 3 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(4, 3));
	EXPECT_EQ(action->getText(), "line\nthirdline");
}

TEST(ActionTests, WriteEraseCutFromCenterTest) {
	COORD startPos = COORD{ 14, 4 };
	COORD otherStartPos = COORD{ 4, 5 };
	COORD otherEndPos = COORD{ 19, 4 };
	auto splitText = getSplitText();
	auto otherSplitText = std::vector<std::string>{ "line", "seco" };

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(14, 4));
	EXPECT_EQ(action->getText(), "firstndline\nthirdline");
}

TEST(ActionTests, WriteEraseCutAllTest) {
	COORD startPos = COORD{ 14, 4 };
	COORD otherStartPos = COORD{ 9, 6 };
	COORD otherEndPos = COORD{ 14, 4 };
	auto splitText = getSplitText();
	auto otherSplitText = getSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(14, 4));
	EXPECT_EQ(action->getText(), "");
}

TEST(ActionTests, WriteEraseCutAllButMoreTest) {
	COORD startPos = COORD{ 14, 4 };
	COORD otherStartPos = COORD{ 12, 6 };
	COORD otherEndPos = COORD{ 11, 4 };
	auto splitText = getSplitText();
	auto otherSplitText = std::vector<std::string>{ "123firstline", "secondline", "thirdline432" };

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getText(), "");
}

TEST(ActionTests, WriteEraseCutFromRightSideTest) {
	COORD startPos = COORD{ 14, 4 };
	COORD otherStartPos = COORD{ 6, 7 };
	COORD otherEndPos = COORD{ 16, 5 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(14, 4));
	EXPECT_EQ(action->getText(), "firstline\nsecondline");
}

TEST(ActionTests, WriteEraseBelowOneLineTest) {
	COORD startPos = COORD{ 10, 4 };
	COORD otherStartPos = COORD{ 6, 2 };
	COORD otherEndPos = COORD{ 4, 0 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(10, 2));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, WriteEraseBelowOneLineOnlyOneLineTest) {
	COORD startPos = COORD{ 10, 4 };
	COORD otherStartPos = COORD{ 14, 2 };
	COORD otherEndPos = COORD{ 3, 2 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherShortSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(10, 4));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, EraseWriteAffectFromLeftSideNoChangesTest) {
	COORD startPos = COORD{ 9, 2 };
	COORD endPos = COORD{ 5, 0 };
	COORD otherStartPos = COORD{ 5, 2 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(5, 0));
	EXPECT_EQ(action->getRightPos(), COORD(9, 2));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, EraseWriteAffectFromLeftSideTest) {
	COORD startPos = COORD{ 9, 4 };
	COORD endPos = COORD{ 5, 2 };
	COORD otherStartPos = COORD{ 4, 2 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(7, 4));
	EXPECT_EQ(action->getRightPos(), COORD(9, 6));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, EraseWriteAffectFromRightSideTest) {
	COORD startPos = COORD{ 9, 2 };
	COORD endPos = COORD{ 5, 0 };
	COORD otherStartPos = COORD{ 10, 2 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getRightPos(), COORD(9, 2));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, EraseWriteAffectBelowOneLineTest) {
	COORD startPos = COORD{ 9, 6 };
	COORD endPos = COORD{ 5, 4 };
	COORD otherStartPos = COORD{ 5, 0 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getRightPos(), COORD(9, 8));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, EraseWriteAffectBelowOneLineOnlyOneLineTest) {
	COORD startPos = COORD{ 9, 6 };
	COORD endPos = COORD{ 5, 4 };
	COORD otherStartPos = COORD{ 5, 0 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherShortSplitText();

	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getRightPos(), COORD(9, 6));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, EraseEraseAffectFromLeftSideTest) {
	COORD startPos = COORD{ 9, 10 };
	COORD endPos = COORD{ 5, 5 };
	COORD otherStartPos = COORD{ 5, 5 };
	COORD otherEndPos = COORD{ 6, 3 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(6, 3));
	EXPECT_EQ(action->getRightPos(), COORD(9, 8));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, EraseEraseAffectFromRightSideTest) {
	COORD startPos = COORD{ 9, 5 };
	COORD endPos = COORD{ 5, 0 };
	COORD otherStartPos = COORD{ 11, 8 };
	COORD otherEndPos = COORD{ 6, 3 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getLeftPos(), COORD(5, 0));
	EXPECT_EQ(action->getRightPos(), COORD(9, 5));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, EraseEraseAffectBelowOneLineTest) {
	COORD startPos = COORD{ 9, 16 };
	COORD endPos = COORD{ 5, 11 };
	COORD otherStartPos = COORD{ 11, 8 };
	COORD otherEndPos = COORD{ 6, 3 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getRightPos(), COORD(9, 11));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, EraseEraseAffectBelowOneLineOnlyOneLineTest) {
	COORD startPos = COORD{ 9, 16 };
	COORD endPos = COORD{ 5, 11 };
	COORD otherStartPos = COORD{ 11, 8 };
	COORD otherEndPos = COORD{ 6, 8 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	otherAction->affect(*action);

	EXPECT_EQ(action->getRightPos(), COORD(9, 16));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdline");
}

TEST(ActionTests, WriteMergeTest) {
	COORD startPos = COORD{ 5, 0 };
	COORD otherStartPos = COORD{ 9, 2 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<WriteAction>(startPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<WriteAction>(otherStartPos, otherSplitText, nullptr, nullptr);
	EXPECT_TRUE(action->tryMerge(std::move(otherAction)));

	EXPECT_EQ(action->getLeftPos(), COORD(5, 0));
	EXPECT_EQ(action->getRightPos(), COORD(6, 4));
	EXPECT_EQ(action->getText(), "firstline\nsecondline\nthirdlinemicrophones\nheadphones\nmouses");
}

TEST(ActionTests, EraseMergeTest) {
	COORD startPos = COORD{ 9, 16 };
	COORD endPos = COORD{ 11, 14 };
	COORD otherStartPos = COORD{ 11, 14 };
	COORD otherEndPos = COORD{ 6, 12 };
	auto splitText = getSplitText();
	auto otherSplitText = getOtherSplitText();

	ActionPtr action = std::make_unique<EraseAction>(startPos, endPos, splitText, nullptr, nullptr);
	auto otherAction = std::make_unique<EraseAction>(otherStartPos, otherEndPos, otherSplitText, nullptr, nullptr);
	EXPECT_TRUE(action->tryMerge(std::move(otherAction)));

	EXPECT_EQ(action->getRightPos(), COORD(9, 16));
	EXPECT_EQ(action->getLeftPos(), COORD(6, 12));
	EXPECT_EQ(action->getText(), "microphones\nheadphones\nmousesfirstline\nsecondline\nthirdline");
}
