#include "pch.h"
#include "framer.h"

const std::string testStr = "test string";

msg::Buffer prepTestMsg() {
	msg::Buffer msg{16};
	msg::serializeTo(msg, 0, testStr);
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
	firstBigBuffer.add(&msgWithSize, 0, 10);
	secondBigBuffer.add(&msgWithSize, 10, msgWithSize.size - 10);
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
	firstBigBuffer.add(&msgWithSize, 0, 2);
	secondBigBuffer.add(&msgWithSize, 2, msgWithSize.size - 2);
	msg::serializeTo(secondBigBuffer, 0, msgWithSize);

	testMsgExtraction(framer, firstBigBuffer, 2);
	testMsgExtraction(framer, secondBigBuffer, 2);
}