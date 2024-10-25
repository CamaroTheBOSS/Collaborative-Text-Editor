#include "pch.h"

#include <string>
#include <fstream>

#include "logger.h"
#include "messages.h"

const std::string testLogFilePath = "test.log";

bool testLogFileFirstLine(const std::string& desiredStr) {
	std::ifstream file(testLogFilePath, std::ios::in);
	if (!file) {
		return false;
	}
	std::string firstLine;
	std::getline(file, firstLine);
	auto startPos = firstLine.find("]") + 2;
	auto relevantPart = firstLine.substr(startPos, firstLine.size() - startPos - 1);
	return relevantPart == desiredStr;
}

TEST(LoggerTests, HappyLogTest) {
	logs::Logger logger(testLogFilePath);
	logger.logInfo("Test log");
	EXPECT_TRUE(testLogFileFirstLine("INFO Test log"));
}

TEST(LoggerTests, LogIntTest) {
	logs::Logger logger(testLogFilePath);
	logger.logInfo(1);
	EXPECT_TRUE(testLogFileFirstLine("INFO 1"));
}

TEST(LoggerTests, LogUnsignedIntTest) {
	logs::Logger logger(testLogFilePath);
	unsigned int n = 3;
	logger.logInfo(n);
	EXPECT_TRUE(testLogFileFirstLine("INFO 3"));
}

TEST(LoggerTests, LogOneByteIntTest) {
	logs::Logger logger(testLogFilePath);
	msg::OneByteInt n = 7;
	logger.logInfo(n);
	EXPECT_TRUE(testLogFileFirstLine("INFO 7"));
}