#include "serializer.h"

msg::Buffer Serializer::makeConnectResponse(const ServerSiteDocument& doc, const int userIdx, const msg::ForwardConnect& msg) {
	return makeConnectResponseImpl(doc, msg.version, userIdx);
}

msg::Buffer Serializer::makeConnectResponse(const ServerSiteDocument& doc, const int userIdx, const msg::Connect& msg) {
	return makeConnectResponseImpl(doc, msg.version, userIdx);
}

msg::Buffer Serializer::makeConnectResponseImpl(const ServerSiteDocument& doc, const msg::OneByteInt version, const int userIdx) {
	std::vector<unsigned int> cursorPositions;
	for (const auto& cursorPos : doc.getCursorPositions()) {
		cursorPositions.push_back(static_cast<unsigned int>(cursorPos.X));
		cursorPositions.push_back(static_cast<unsigned int>(cursorPos.Y));
	}
	std::string docText = doc.getText();
	msg::Buffer buffer{static_cast<int>(30 + docText.size() + 8 * cursorPositions.size())};
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	msg::serializeTo(buffer, 0, msg::Type::sync, version, userBuff, std::move(docText), std::move(cursorPositions));
	return buffer;
}


msg::Buffer Serializer::makeDisconnectResponse(const int userIdx, const msg::Disconnect& msg) {
	msg::Buffer buffer{30};
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	msg::serializeTo(buffer, 0, msg::Type::disconnect, msg.version, userBuff);
	return buffer;
}

msg::Buffer Serializer::makeWriteResponse(const COORD& startPos, const int userIdx, const msg::Write& msg) {
	msg::Buffer buffer{static_cast<int>(30 + msg.text.size())};
	unsigned int cursorX = startPos.X;
	unsigned int cursorY = startPos.Y;
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	msg::serializeTo(buffer, 0, msg.type, msg.version, userBuff, msg.text, cursorX, cursorY);
	return buffer;
}

msg::Buffer Serializer::makeEraseResponse(const COORD& startPos, const int userIdx, const msg::Erase& msg) {
	msg::Buffer buffer{30};
	unsigned int cursorX = startPos.X;
	unsigned int cursorY = startPos.Y;
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	msg::serializeTo(buffer, 0, msg.type, msg.version, userBuff, msg.eraseSize, cursorX, cursorY);
	return buffer;
}

msg::Buffer Serializer::makeMoveResponse(const ServerSiteDocument& doc, const int userIdx, const msg::MoveHorizontal& msg) {
	return makeMoveResponseImpl(doc, msg.type, msg.version, userIdx, msg.withSelect);
}

msg::Buffer Serializer::makeMoveResponse(const ServerSiteDocument& doc, const int userIdx, const msg::MoveVertical& msg) {
	return makeMoveResponseImpl(doc, msg.type, msg.version, userIdx, msg.withSelect);
}

msg::Buffer Serializer::makeMoveResponse(const ServerSiteDocument& doc, const int userIdx, const msg::MoveTo& msg) {
	return makeMoveResponseImpl(doc, msg.type, msg.version, userIdx, false);
}

msg::Buffer Serializer::makeMoveResponse(const ServerSiteDocument& doc, const int userIdx, const msg::MoveSelectAll& msg) {
	return makeMoveResponseImpl(doc, msg.type, msg.version, userIdx, true);
}

msg::Buffer Serializer::makeMoveResponseImpl(const ServerSiteDocument& doc, const msg::Type type, const msg::OneByteInt version, const int userIdx, const bool withSelect) {
	msg::Buffer buffer{30};
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	auto cursorPos = doc.getCursorPos(userIdx);
	unsigned int cursorX = cursorPos.X;
	unsigned int cursorY = cursorPos.Y;
	auto anchor = doc.getCursorSelectionAnchor(userIdx);
	unsigned int anchorX = anchor.value_or(COORD{ 0, 0 }).X;
	unsigned int anchorY = anchor.value_or(COORD{ 0, 0 }).Y;
	msg::serializeTo(buffer, 0, type, version, userBuff, cursorX, cursorY, withSelect, anchorX, anchorY);
	return buffer;
}

msg::Buffer Serializer::makeReplaceResponse(const int userIdx, const msg::Replace& msg) {
	int bufferSize = 16 * msg.segments.size() + msg.text.size() + 10;
	msg::Buffer buffer{ bufferSize };
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	msg::serializeTo(buffer, 0, msg::Type::replace, msg.version, userBuff, msg.text, msg.segments);
	return buffer;
}