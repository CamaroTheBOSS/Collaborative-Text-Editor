#pragma once
#include <vector>

#include "messages.h"

using Messages = std::vector<msg::Buffer>;

class Framer {
public:
	Framer(const int capacity);
	Messages extractMessages(msg::Buffer& recvBuff);
private:
	enum class Mode {frame, drop};

	Messages drop(int startPos, msg::Buffer& recvBuff, Messages& messages);
	Messages frame(int startPos, msg::Buffer& recvBuff, Messages& messages);

	Mode mode = Mode::frame;
	msg::Buffer workBuff;
	unsigned int neededSymbols = 0;
};