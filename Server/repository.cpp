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
	case msg::Type::sync:
		return connectUserToDoc(client, buffer);
	case msg::Type::disconnect:
		return disconnectUserFromDoc(client, buffer);
	case msg::Type::write:
		return write(client, buffer);
	case msg::Type::erase:
		return erase(client, buffer);
	case msg::Type::moveHorizontal:
		return moveHorizontal(client, buffer);
	case msg::Type::moveVertical:
		return moveVertical(client, buffer);
	}
	assert(false && "Unrecognized msg type. Aborting...");
	return Response{ buffer, {}, msg::Type::error };
}

int Repository::findClient(SOCKET client) {
	std::scoped_lock lock{connectedClientsLock};
	int cursor = -1;
	for (int i = 0; i < connectedClients.size(); i++) {
		if (connectedClients[i] == client) {
			cursor = i;
			break;
		}
	}
	return cursor;
}

Response Repository::connectUserToDoc(SOCKET client, msg::Buffer& buffer) {
	msg::Connect msg;
	msg::parse(buffer, 1, msg.version);
	logger.logDebug("Thread", std::this_thread::get_id(), "connected new user to document!");
	std::scoped_lock lock{connectedClientsLock, docLock};
	msg::OneByteInt cursor = connectedClients.size();
	connectedClients.push_back(client);
	doc.addCursor();
	buffer.clear();
	msg::serializeTo(buffer, 0, msg::Type::sync, msg.version, cursor, doc.getText());
	return Response{ buffer, { connectedClients }, msg::Type::sync };
}

Response Repository::disconnectUserFromDoc(SOCKET client, msg::Buffer& buffer) {
	msg::Disconnect msg;
	msg::parse(buffer, 1, msg.version);
	int cursor = findClient(client);
	std::scoped_lock lock{connectedClientsLock, docLock};
	if (cursor == -1) {
		logger.logDebug("Error when trying to disconnect user! Cursor not found!!!");
	}
	else {
		connectedClients.erase(connectedClients.cbegin() + cursor);
		doc.eraseCursor(cursor);
	}
	buffer.clear();
	msg::serializeTo(buffer, 0, msg::Type::disconnect, msg.version, cursor);
	return Response{ buffer, connectedClients, msg::Type::disconnect };
}

Response Repository::masterNotification(SOCKET client, msg::Buffer& buffer) const {
	logger.logDebug("Thread", std::this_thread::get_id(), "got new connection!");
	return Response{ buffer, {} };
}

Response Repository::write(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::Write{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.text);
	int cursor = findClient(client);
	std::scoped_lock lock{docLock};
	doc.write(cursor, msg.text);
	logger.logInfo("cursor", cursor, "wrote '" + msg.text + "' to document");
	buffer.clear();
	auto cursorBuff = static_cast<msg::OneByteInt>(cursor);
	msg::serializeTo(buffer, 0, msg.type, msg.version, cursorBuff, msg.text);
	return Response{ buffer, connectedClients, msg::Type::write };
}

Response Repository::erase(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::Erase{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.eraseSize);
	int cursor = findClient(client);
	std::scoped_lock lock{docLock};
	doc.erase(cursor, msg.eraseSize);
	logger.logInfo("cursor", cursor, "erased", msg.eraseSize, "letters from document");
	buffer.clear();
	auto cursorBuff = static_cast<msg::OneByteInt>(cursor);
	msg::serializeTo(buffer, 0, msg.type, msg.version, cursorBuff, msg.eraseSize);
	return Response{ buffer, connectedClients, msg::Type::erase };
}

Response Repository::moveHorizontal(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::MoveHorizontal{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.side);
	int cursor = findClient(client);
	std::scoped_lock lock{docLock};
	COORD newCursorPos = doc.getCursorPos(cursor);
	if (msg.side == msg::MoveSide::left) {
		newCursorPos = doc.moveCursorLeft(cursor);
		logger.logInfo("cursor", cursor, "moved left");
	}
	else if (msg.side == msg::MoveSide::right) {
		newCursorPos = doc.moveCursorRight(cursor);
		logger.logInfo("cursor", cursor, "moved right");
	}
	else {
		logger.logError("Invalid MoveSide parameter in MoveHorizontal");
		return Response{ buffer, {}, msg::Type::error };
	}
	buffer.clear();
	auto cursorBuff = static_cast<msg::OneByteInt>(cursor);
	unsigned int cursorX = newCursorPos.X;
	unsigned int cursorY = newCursorPos.Y;
	msg::serializeTo(buffer, 0, msg.type, msg.version, cursorBuff, cursorX, cursorY);
	return Response{ buffer, connectedClients, msg::Type::moveHorizontal };
}

Response Repository::moveVertical(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::MoveVertical{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.side, msg.clientWidth);
	int cursor = findClient(client);
	std::scoped_lock lock{docLock};
	COORD newCursorPos = doc.getCursorPos(cursor);
	if (msg.side == msg::MoveSide::up) {
		newCursorPos = doc.moveCursorUp(cursor, msg.clientWidth);
		logger.logInfo("cursor", cursor, "moved up");
	}
	else if (msg.side == msg::MoveSide::down) {
		newCursorPos = doc.moveCursorDown(cursor, msg.clientWidth);
		logger.logInfo("cursor", cursor, "moved down");
	}
	else {
		logger.logError("Invalid MoveSide parameter in MoveHorizontal");
		return Response{ buffer, {}, msg::Type::error };
	}
	buffer.clear();
	auto cursorBuff = static_cast<msg::OneByteInt>(cursor);
	unsigned int cursorX = newCursorPos.X;
	unsigned int cursorY = newCursorPos.Y;
	msg::serializeTo(buffer, 0, msg.type, msg.version, cursorBuff, cursorX, cursorY);
	return Response{ buffer, connectedClients, msg::Type::moveVertical };
}