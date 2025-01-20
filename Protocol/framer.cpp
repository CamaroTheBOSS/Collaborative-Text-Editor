#include "framer.h"

void saveBuff(const msg::Buffer& src, std::vector<msg::Buffer>& dst, const int maxSize) {
	dst.emplace_back(msg::Buffer{ src.size });
	dst[dst.size() - 1].add(&src, 0, src.size);
	if (dst.size() > maxSize) {
		dst.erase(dst.cbegin(), dst.cbegin() + 1);
	}
}

Framer::Framer(const int capacity) :
	msgBuff(capacity) {}

Messages Framer::extractMessages(msg::Buffer& recvBuff) {
	Messages messages;
	if (state == State::length) {
		extractLength(0, recvBuff, messages);
	}
	else if (state == State::msg) {
		extractMsg(0, recvBuff, messages);
	}
	saveBuff(recvBuff, _prevBuffs, maxPrevBuffLen);
	return messages;
}


void Framer::extractLength(int head, msg::Buffer& recvBuff, Messages& messages) {
	int remainingSymbols = recvBuff.size - head;
	if (remainingSymbols <= 0) {
		return;
	}

	if (lenBuff.size < 4) {
		int lenRemainingSymbols = (std::min)(4 - lenBuff.size, remainingSymbols);
		lenBuff.add(&recvBuff, head, lenRemainingSymbols);
		head += lenRemainingSymbols;
	}

	if (lenBuff.size >= 4) {
		msg::parse(lenBuff, 0, neededSymbols);
		msgBuff.reserveIfNeeded(neededSymbols);
		assert(neededSymbols >= 2 && neededSymbols <= msgBuff.capacity);
		saveBuff(lenBuff, _prevMsgLengths, maxPrevMsgBuffLen);
		lenBuff.clear();
		state = State::msg;
		return extractMsg(head, recvBuff, messages);
	}
	return;
}

void Framer::extractMsg(int head, msg::Buffer& recvBuff, Messages& messages) {
	int remainingSymbols = recvBuff.size - head;
	if (remainingSymbols <= 0) {
		return;
	}

	int symbolsToCopy = (std::min)(remainingSymbols, static_cast<int>(neededSymbols));
	msgBuff.add(&recvBuff, head, symbolsToCopy);
	neededSymbols -= symbolsToCopy;
	head += symbolsToCopy;

	if (neededSymbols == 0) {
		messages.emplace_back(msg::Buffer{msgBuff.size});
		messages[messages.size() - 1].add(&msgBuff);
		saveBuff(msgBuff, _prevMsgs, maxPrevMsgBuffLen);
		msgBuff.clear();
		state = State::length;
		return extractLength(head, recvBuff, messages);
	}
	return;
}
