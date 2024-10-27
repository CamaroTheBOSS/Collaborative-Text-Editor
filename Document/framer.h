#pragma once
#include <vector>

#include "messages.h"

class Framer {
public:
	Framer(const int capacity);
	std::vector<msg::Buffer> extractMessages(msg::Buffer& recvBuff);
private:
	msg::Buffer workBuff;
	unsigned int neededSymbols = 0;
};