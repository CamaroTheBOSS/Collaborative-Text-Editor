#pragma once
#include <vector>

#include "messages.h"

using Messages = std::vector<msg::Buffer>;

class Framer {
public:
	Framer(const int capacity);
	Messages extractMessages(msg::Buffer& recvBuff);
private:
	void extractLength(int head, msg::Buffer& recvBuff, Messages& messages);
	void extractMsg(int head, msg::Buffer& recvBuff, Messages& messages);
	enum class State {length, msg, drop};

	State state = State::length;
	msg::Buffer msgBuff;
	msg::Buffer lenBuff{4};
	unsigned int neededSymbols = 0;

	// debug buffers
	int maxPrevBuffLen = 5;
	int maxPrevMsgBuffLen = 100;
	std::vector<msg::Buffer> _prevBuffs;
	std::vector<msg::Buffer> _prevMsgs;
	std::vector<msg::Buffer> _prevMsgLengths;
};