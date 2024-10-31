#include "framer.h"

Framer::Framer(const int capacity) :
	workBuff(capacity) {}

Messages Framer::extractMessages(msg::Buffer& recvBuff) {
	Messages messages;
	if (mode == Mode::frame) {
		return frame(0, recvBuff, messages);
	}
	return drop(0, recvBuff, messages);
}

Messages Framer::drop(int head, msg::Buffer& recvBuff, Messages& messages) {
	int remainingSymbols = recvBuff.size - head;
	if (neededSymbols < remainingSymbols) {
		head += neededSymbols;
		neededSymbols = 0;
		mode = Mode::frame;
		workBuff.clear();
		return frame(head, recvBuff, messages);
	}
	neededSymbols -= remainingSymbols;
	return messages;
}


Messages Framer::frame(int head, msg::Buffer& recvBuff, Messages& messages) {
	int remainingSymbols = recvBuff.size - head;
	if (remainingSymbols == 0) {
		return messages;
	}
	if (remainingSymbols < 4) {
		workBuff.add(&recvBuff, head, remainingSymbols);
		return messages;
	}

	int msgSizeRemainingSymbols = 4 - workBuff.size;
	if (msgSizeRemainingSymbols > 0) {
		workBuff.add(&recvBuff, head, msgSizeRemainingSymbols);
		msg::parse(workBuff, 0, neededSymbols);
		head += msgSizeRemainingSymbols;
		remainingSymbols -= msgSizeRemainingSymbols;
	}
	if (neededSymbols > workBuff.capacity - 4) {
		mode = Mode::drop;
		workBuff.clear();
		return drop(head, recvBuff, messages);
	}

	if (remainingSymbols < neededSymbols) {
		workBuff.add(&recvBuff, head, remainingSymbols);
		neededSymbols -= remainingSymbols;
		return messages;
	}

	workBuff.add(&recvBuff, head, neededSymbols);
	messages.emplace_back(msg::Buffer{static_cast<int>(workBuff.size) - 4});
	messages[messages.size() - 1].add(&workBuff, 4, workBuff.size - 4);

	head += neededSymbols;
	neededSymbols = 0;
	workBuff.clear();
	return frame(head, recvBuff, messages);
}
