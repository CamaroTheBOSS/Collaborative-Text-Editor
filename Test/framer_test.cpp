#include "pch.h"
#include "framer.h"

const std::string strWithPrimarSize = "unbeliveable very long and large";
const std::string testStr = "test string";
const std::string largeTestStr = R"(
	unbeliveable very long and large test string what the hell is that I cannot belive that :O.
	unbeliveable very long and large test string what the hell is that I cannot belive that :O.
	unbeliveable very long and large test string what the hell is that I cannot belive that :O.
	unbeliveable very long and large test string what the hell is that I cannot belive that :O.
	unbeliveable very long and large test string what the hell is that I cannot belive that :O.
	unbeliveable very long and large test string what the hell is that I cannot belive that :O.
	unbeliveable very long and large test string what the hell is that I cannot belive that :O.
	unbeliveable very long and large test string what the hell is that I cannot belive that :O.
)";

msg::Buffer prepPrimarTestMsg() {
	msg::Buffer msg{37};
	msg::serializeTo(msg, 0, strWithPrimarSize);
	msg::Buffer msgWithSize = msg::enrich(msg);
	return msgWithSize;
}

msg::Buffer prepTestMsg() {
	msg::Buffer msg{16};
	msg::serializeTo(msg, 0, testStr);
	msg::Buffer msgWithSize = msg::enrich(msg);
	return msgWithSize;
}

msg::Buffer prepLargeTestMsg() {
	msg::Buffer msg{1000};
	msg::serializeTo(msg, 0, largeTestStr);
	msg::Buffer msgWithSize = msg::enrich(msg);
	return msgWithSize;
}

std::vector<std::string> extractStrings(Framer& framer, msg::Buffer& buffer ) {
	std::vector<std::string> messages;
	auto separatedMessages = framer.extractMessages(buffer);
	for (auto& message : separatedMessages) {
		std::string parsedStr;
		msg::parse(message, 0, parsedStr);
		messages.emplace_back(parsedStr);
	}
	return messages;
}

void testMsgs(const std::vector<std::string>& actual, const std::vector<std::string> expected) {
	EXPECT_EQ(actual.size(), expected.size());
	if (actual.size() != expected.size()) {
		return;
	}
	for (int i = 0; i < actual.size(); i++) {
		EXPECT_EQ(actual[i], expected[i]);
	}
}

TEST(FramerTests, ExtractingThreeMessagesTest) {
	Framer framer{ 128 };
	msg::Buffer bigBuffer{48};
	auto msgWithSize = prepTestMsg();
	msg::serializeTo(bigBuffer, 0, msgWithSize, msgWithSize, msgWithSize);
	auto msgs = extractStrings(framer, bigBuffer);
	testMsgs(msgs, { testStr, testStr, testStr });
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
	auto firstMsgs = extractStrings(framer, firstBigBuffer);
	auto secondMsgs = extractStrings(framer, secondBigBuffer);
	testMsgs(firstMsgs, { testStr, testStr });
	testMsgs(secondMsgs, { testStr, testStr });
}

TEST(FramerTests, ExtractingIncompleteMessageLastFourBytesTest) {
	Framer framer{ 1024 };
	int nMessages = 2;
	auto msg = prepTestMsg();
	for (int i = 0; i < 4; i++) {
		msg::Buffer firstBigBuffer{(nMessages + 1) * 16 - 1 - i};
		msg::Buffer secondBigBuffer{64};
		for (int j = 0; j < nMessages; j++) {
			firstBigBuffer.add(&msg);
		}
		int anchor = firstBigBuffer.capacity - firstBigBuffer.size;
		firstBigBuffer.add(&msg, 0, anchor);
		secondBigBuffer.add(&msg, anchor, msg.size - anchor);
		msg::serializeTo(secondBigBuffer, msg.size - anchor, msg, msg);
		auto firstMsgs = extractStrings(framer, firstBigBuffer);
		auto secondMsgs = extractStrings(framer, secondBigBuffer);
		testMsgs(firstMsgs, std::vector<std::string>(nMessages, testStr));
		testMsgs(secondMsgs, std::vector<std::string>(3, testStr));
	}
	// there was a bug where in this specific test case neededSymbols were read wrongly what was broking whole Framer
}

TEST(FramerTests, ExtractingIncompleteMessageLastFourBytesWithFinishingInSeparateBufferTest) {
	Framer framer{ 1024 };
	int nMessages = 2;
	auto msg = prepTestMsg();
	for (int i = 0; i < 4; i++) {
		msg::Buffer firstBigBuffer{(nMessages + 1) * 16 - 1 - i};
		msg::Buffer secondBigBuffer{64};
		msg::Buffer thirdBigBuffer{64};
		for (int j = 0; j < nMessages; j++) {
			firstBigBuffer.add(&msg);
		}
		int anchor = firstBigBuffer.capacity - firstBigBuffer.size;
		firstBigBuffer.add(&msg, 0, anchor);
		secondBigBuffer.add(&msg, anchor, msg.size - anchor);
		msg::serializeTo(thirdBigBuffer, 0, msg, msg);
		auto firstMsgs = extractStrings(framer, firstBigBuffer);
		auto secondMsgs = extractStrings(framer, secondBigBuffer);
		auto thirdMsgs = extractStrings(framer, thirdBigBuffer);
		testMsgs(firstMsgs, std::vector<std::string>(nMessages, testStr));
		testMsgs(secondMsgs, std::vector<std::string>(1, testStr));
		testMsgs(thirdMsgs, std::vector<std::string>(2, testStr));
	}
	// there was a bug where in this specific test case neededSymbols were read wrongly what was broking whole Framer
}


TEST(FramerTests, ExtractingMessageWithIncompleteLengthTest) {
	Framer framer{ 1024 };
	int nMessages = 5;
	auto msg = prepTestMsg();
	auto largeMsg = prepLargeTestMsg();
	for (int i = 0; i < 4; i++) {
		msg::Buffer firstBigBuffer{nMessages * 16 + 1 + i};
		msg::Buffer secondBigBuffer{1000};
		for (int j = 0; j < nMessages; j++) {
			firstBigBuffer.add(&msg);
		}
		int anchor = firstBigBuffer.capacity - firstBigBuffer.size;
		firstBigBuffer.add(&largeMsg, 0, anchor);
		secondBigBuffer.add(&largeMsg, anchor, largeMsg.size - anchor);
		msg::serializeTo(secondBigBuffer, largeMsg.size - anchor, msg, msg);
		auto firstMsgs = extractStrings(framer, firstBigBuffer);
		auto secondMsgs = extractStrings(framer, secondBigBuffer);
		testMsgs(firstMsgs, std::vector<std::string>(nMessages, testStr));
		testMsgs(secondMsgs, { largeTestStr, testStr, testStr });
	}
}

TEST(FramerTests, ContinousExtractionTest) {
	Framer framer{ 128 };
	constexpr int bufferSize = 10000;
	constexpr int fragmentation = 128;	

	auto msg = prepPrimarTestMsg();
	const int expectedNMsgs = bufferSize / msg.size;
	msg::Buffer continousBuffer{bufferSize};
	for (int i = 0; i < expectedNMsgs; i++) {
		continousBuffer.add(&msg);
	}
	
	int nMsgs = 0;
	int head = 0;
	int remainingSymbols = 0;
	do {
		msg::Buffer buffer{fragmentation};
		remainingSymbols = continousBuffer.size - head;
		int nSymbols = (std::min)(remainingSymbols, fragmentation);
		buffer.add(&continousBuffer, head, nSymbols);
		auto extractedMsgs = extractStrings(framer, buffer);
		nMsgs += extractedMsgs.size();
		for (const auto& msg : extractedMsgs) {
			EXPECT_EQ(msg, strWithPrimarSize);
		}
		head += nSymbols;
	} while (remainingSymbols);
	EXPECT_EQ(nMsgs, expectedNMsgs);
}