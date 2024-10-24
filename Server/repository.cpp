#include <thread>

#include "repository.h"
#include "logging.h"

Response Repository::process(SOCKET client, msg::Buffer& buffer) {
	if (buffer.size == 1) {
		return masterNotification(client, buffer);
	}
	msg::Type type;
	msg::OneByteInt version;
	msg::parse(buffer, 0, type, version);
	switch (type) {
	case msg::Type::write:
		return write(client, buffer);
	case msg::Type::erase:
		return erase(client, buffer);
	case msg::Type::moveHorizontal:
		return moveHorizontal(client, buffer);
	case msg::Type::moveVertical:
		return moveVertical(client, buffer);
	}
	logger.logError("Unrecognized message type ", type);
	return Response{ buffer, {} };
}

void Repository::connectUser(SOCKET client) {
	std::scoped_lock lock{clientToCursorLock};
	clientToCursor.try_emplace(client, connectedClients.size());
	connectedClients.push_back(client);
}

Response Repository::masterNotification(SOCKET client, msg::Buffer& buffer) {
	logger.logDebug("Thread ", std::this_thread::get_id(), " got new connection!");
	return Response{ buffer, {} };
}

Response Repository::write(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::Write{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.text);
	std::scoped_lock lock{clientToCursorLock, docLock};
	int cursor = clientToCursor[client];
	doc.write(cursor, msg.text);
	logger.logInfo("cursor ", cursor, " wrote '" + msg.text + "' to document");
	buffer.clear();
	auto cursorBuff = static_cast<msg::OneByteInt>(cursor);
	msg::serializeTo(buffer, 0, msg.type, msg.version, cursorBuff, msg.text);
	return Response{ buffer, connectedClients };
}

Response Repository::erase(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::Erase{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.eraseSize);
	std::scoped_lock lock{clientToCursorLock, docLock};
	int cursor = clientToCursor[client];
	doc.erase(cursor, msg.eraseSize);
	logger.logInfo("cursor ", cursor, " erased ", msg.eraseSize, " letters from document");
	buffer.clear();
	auto cursorBuff = static_cast<msg::OneByteInt>(cursor);
	msg::serializeTo(buffer, 0, msg.type, msg.version, cursorBuff, msg.eraseSize);
	return Response{ buffer, connectedClients };
}

Response Repository::moveHorizontal(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::MoveHorizontal{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.side);
	std::scoped_lock lock{clientToCursorLock, docLock};
	int cursor = clientToCursor[client];
	COORD newCursorPos = doc.getCursorPos(cursor);
	if (msg.side == msg::MoveSide::left) {
		newCursorPos = doc.moveCursorLeft(cursor);
		logger.logInfo("cursor ", cursor, " moved left");
	}
	else if (msg.side == msg::MoveSide::right) {
		newCursorPos = doc.moveCursorRight(cursor);
		logger.logInfo("cursor ", cursor, " moved right");
	}
	else {
		logger.logError("Invalid MoveSide parameter in MoveHorizontal");
		return Response{ buffer, {} };
	}
	buffer.clear();
	auto cursorBuff = static_cast<msg::OneByteInt>(cursor);
	unsigned int cursorX = newCursorPos.X;
	unsigned int cursorY = newCursorPos.Y;
	msg::serializeTo(buffer, 0, msg.type, msg.version, cursorBuff, cursorX, cursorY);
	return Response{ buffer, connectedClients };
}

Response Repository::moveVertical(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::MoveVertical{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.side, msg.clientWidth);
	std::scoped_lock lock{clientToCursorLock, docLock};
	int cursor = clientToCursor[client];
	COORD newCursorPos = doc.getCursorPos(cursor);
	if (msg.side == msg::MoveSide::up) {
		newCursorPos = doc.moveCursorUp(cursor, msg.clientWidth);
		logger.logInfo("cursor ", cursor, " moved up");
	}
	else if (msg.side == msg::MoveSide::down) {
		newCursorPos = doc.moveCursorDown(cursor, msg.clientWidth);
		logger.logInfo("cursor ", cursor, " moved down");
	}
	else {
		logger.logError("Invalid MoveSide parameter in MoveHorizontal");
		return Response{ buffer, {} };
	}
	buffer.clear();
	auto cursorBuff = static_cast<msg::OneByteInt>(cursor);
	unsigned int cursorX = newCursorPos.X;
	unsigned int cursorY = newCursorPos.Y;
	msg::serializeTo(buffer, 0, msg.type, msg.version, cursorBuff, cursorX, cursorY);
	return Response{ buffer, connectedClients };
}