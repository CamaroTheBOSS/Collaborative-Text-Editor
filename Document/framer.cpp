#include "framer.h"

Framer::Framer(const int capacity) :
	workBuff(capacity) {}

std::vector<msg::Buffer> Framer::extractMessages(msg::Buffer& recvBuff) {
	int head = 0;
	std::vector<msg::Buffer> messages;
	int remainingSymbols = 0;
	while ((remainingSymbols = recvBuff.size - head) > 0) {
		if (remainingSymbols < 4) {
			workBuff.add(&recvBuff, head, remainingSymbols);
			break;
		}

		int msgSizeRemainingSymbols = 4 - workBuff.size;
		if (msgSizeRemainingSymbols > 0) {
			workBuff.add(&recvBuff, head, msgSizeRemainingSymbols);
			msg::parse(workBuff, 0, neededSymbols);
			head += msgSizeRemainingSymbols;
			remainingSymbols -= msgSizeRemainingSymbols;
		}
			
		if (remainingSymbols < neededSymbols) {
			workBuff.add(&recvBuff, head, remainingSymbols);
			neededSymbols -= remainingSymbols;
			break;
		}

		workBuff.add(&recvBuff, head, neededSymbols);
		messages.emplace_back(msg::Buffer{static_cast<int>(workBuff.size) - 4});
		messages[messages.size() - 1].add(&workBuff, 4, workBuff.size - 4);

		head += neededSymbols;
		neededSymbols = 0;
		workBuff.clear();
	}
	return messages;
}