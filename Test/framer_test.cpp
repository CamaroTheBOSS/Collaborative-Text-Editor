#include "pch.h"
#include "framer.h"

const std::string testStr = "test string";
const std::string largeTestStr = "unbeliveable very large test string what the hell is that";

msg::Buffer prepTestMsg() {
	msg::Buffer msg{12};
	msg::serializeTo(msg, 0, testStr);
	msg::Buffer msgWithSize = msg::enrich(msg);
	return msgWithSize;
}

msg::Buffer prepTestLargeMsg() {
	msg::Buffer msg{62};
	msg::serializeTo(msg, 0, largeTestStr);
	msg::Buffer msgWithSize = msg::enrich(msg);
	return msgWithSize;
}

void testMsgExtraction(Framer& framer, msg::Buffer& buffer, const int expectedMsgs ) {
	
	auto separatedMessages = framer.extractMessages(buffer);
	EXPECT_EQ(separatedMessages.size(), expectedMsgs);
	for (auto& message : separatedMessages) {
		std::string parsedStr;
		msg::parse(message, 0, parsedStr);
		EXPECT_EQ(message.size, testStr.size() + 1);
		EXPECT_EQ(parsedStr, testStr);
	}
}

TEST(FramerTests, ExtractingThreeMessagesTest) {
	Framer framer{ 128 };
	msg::Buffer bigBuffer{48};
	auto msgWithSize = prepTestMsg();
	msg::serializeTo(bigBuffer, 0, msgWithSize, msgWithSize, msgWithSize);
	testMsgExtraction(framer, bigBuffer, 3);
}

TEST(FramerTests, ExtractingIncompleteMessageTest) {
	Framer framer{ 128 };
	msg::Buffer firstBigBuffer{42};
	msg::Buffer secondBigBuffer{42};
	auto msgWithSize = prepTestMsg();
	msg::serializeTo(firstBigBuffer, 0, msgWithSize, msgWithSize);
	int anchor = firstBigBuffer.capacity - firstBigBuffer.size;
	firstBigBuffer.add(&msgWithSize, 0, anchor);
	secondBigBuffer.add(&msgWithSize, anchor, msgWithSize.size - anchor);
	msg::serializeTo(secondBigBuffer, 0, msgWithSize);

	testMsgExtraction(framer, firstBigBuffer, 2);
	testMsgExtraction(framer, secondBigBuffer, 2);
}

TEST(FramerTests, ExtractingMessageWithIncompleteLengthTest) {
	Framer framer{ 128 };
	msg::Buffer firstBigBuffer{34};
	msg::Buffer secondBigBuffer{34};
	auto msgWithSize = prepTestMsg();
	msg::serializeTo(firstBigBuffer, 0, msgWithSize, msgWithSize);
	int anchor = firstBigBuffer.capacity - firstBigBuffer.size;
	firstBigBuffer.add(&msgWithSize, 0, anchor);
	secondBigBuffer.add(&msgWithSize, anchor, msgWithSize.size - anchor);
	msg::serializeTo(secondBigBuffer, 0, msgWithSize);

	testMsgExtraction(framer, firstBigBuffer, 2);
	testMsgExtraction(framer, secondBigBuffer, 2);
}

TEST(FramerTests, DropTooLargeMsgTest) {
	Framer framer{ 10 };
	auto msgWithSize = prepTestMsg();
	testMsgExtraction(framer, msgWithSize, 0);
}

TEST(FramerTests, FrameDropFrameTest) {
	Framer framer{ 32 };
	msg::Buffer firstBigBuffer{128};
	auto msg = prepTestMsg();
	auto largeMsg = prepTestLargeMsg();
	msg::serializeTo(firstBigBuffer, 0, msg, msg, largeMsg, msg, msg);

	testMsgExtraction(framer, firstBigBuffer, 4);
}

TEST(FramerTests, FrameDropFrameInTwoBuffersTest) {
	Framer framer{ 32 };
	msg::Buffer firstBigBuffer{64};
	msg::Buffer secondBigBuffer{64};
	auto msg = prepTestMsg();
	auto largeMsg = prepTestLargeMsg();
	msg::serializeTo(firstBigBuffer, 0, msg, msg);
	int anchor = firstBigBuffer.capacity - firstBigBuffer.size;
	firstBigBuffer.add(&largeMsg, 0, anchor);
	secondBigBuffer.add(&largeMsg, anchor, largeMsg.size - anchor);
	msg::serializeTo(secondBigBuffer, 0, msg, msg);

	testMsgExtraction(framer, firstBigBuffer, 2);
	testMsgExtraction(framer, secondBigBuffer, 2);
}

TEST(FramerTests, FrameDropFrameInTwoBuffersIncompleteLengthTest) {
	Framer framer{ 32 };
	msg::Buffer firstBigBuffer{34};
	msg::Buffer secondBigBuffer{128};
	auto msg = prepTestMsg();
	auto largeMsg = prepTestLargeMsg();
	msg::serializeTo(firstBigBuffer, 0, msg, msg);
	int anchor = firstBigBuffer.capacity - firstBigBuffer.size;
	firstBigBuffer.add(&largeMsg, 0, anchor);
	secondBigBuffer.add(&largeMsg, anchor, largeMsg.size - anchor);
	msg::serializeTo(secondBigBuffer, 0, msg, msg);

	testMsgExtraction(framer, firstBigBuffer, 2);
	testMsgExtraction(framer, secondBigBuffer, 2);
}