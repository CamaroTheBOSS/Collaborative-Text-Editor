#include <thread>

#include "repository.h"
#include "logging.h"

Response Repository::process(SOCKET client, msg::Buffer& buffer) {
	msg::Type type;
	msg::OneByteInt version;
	msg::parse(buffer, 0, type, version);
	switch (type) {
	case msg::Type::masterNotification:
		return masterNotification(client, buffer);
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
	case msg::Type::selectAll:
		return moveSelectAll(client, buffer);
	case msg::Type::undo:
	case msg::Type::redo:
		return undoRedo(client, buffer);
	}
	assert(false && "Unrecognized message type, aborting...");
	return Response{ buffer, {}, msg::Type::error };
}

int Repository::findClient(SOCKET client) {
	std::scoped_lock lock{connectedClientsLock};
	int userIdx = -1;
	for (int i = 0; i < connectedClients.size(); i++) {
		if (connectedClients[i] == client) {
			userIdx = i;
			break;
		}
	}
	return userIdx;
}

Response Repository::connectUserToDoc(SOCKET client, msg::Buffer& buffer) {
	msg::Connect msg;
	msg::parse(buffer, 1, msg.version);
	logger.logDebug("Thread", std::this_thread::get_id(), "connected new user to document!");
	std::scoped_lock lock{connectedClientsLock, docLock};
	msg::OneByteInt userIdx = connectedClients.size();
	connectedClients.push_back(client);
	doc.addUser();
	std::vector<unsigned int> cursorPositions;
	for (const auto& cursorPos : doc.getCursorPositions()) {
		cursorPositions.push_back(static_cast<unsigned int>(cursorPos.X));
		cursorPositions.push_back(static_cast<unsigned int>(cursorPos.Y));
	}
	
	buffer.clear();
	std::string docText = doc.getText();
	buffer.reserve(30 + docText.size() + 8 * cursorPositions.size());
	msg::serializeTo(buffer, 0, msg::Type::sync, msg.version, userIdx, std::move(docText), std::move(cursorPositions));
	return Response{ buffer, { connectedClients }, msg::Type::sync };
}

Response Repository::disconnectUserFromDoc(SOCKET client, msg::Buffer& buffer) {
	msg::Disconnect msg;
	msg::parse(buffer, 1, msg.version);
	int userIdx = findClient(client);
	std::scoped_lock lock{connectedClientsLock, docLock};
	if (userIdx == -1) {
		logger.logDebug("Error when trying to disconnect user! Cursor not found!!!");
	}
	else {
		connectedClients.erase(connectedClients.cbegin() + userIdx);
		doc.eraseUser(userIdx);
	}
	buffer.clear();
	buffer.reserve(30);
	msg::serializeTo(buffer, 0, msg::Type::disconnect, msg.version, userIdx);
	return Response{ buffer, connectedClients, msg::Type::disconnect };
}

Response Repository::masterNotification(SOCKET client, msg::Buffer& buffer) const {
	logger.logDebug("Thread", std::this_thread::get_id(), "got new connection!");
	return Response{ buffer, {} };
}

Response Repository::write(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::Write{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.text);
	int userIdx = findClient(client);
	std::scoped_lock lock{docLock};
	COORD cursorPos = doc.getCursorPos(userIdx);
	unsigned int cursorX = cursorPos.X;
	unsigned int cursorY = cursorPos.Y;
	doc.write(userIdx, msg.text);
	logger.logInfo("user", userIdx, "wrote '" + msg.text + "' to document");
	buffer.clear();
	buffer.reserve(30 + msg.text.size());
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	msg::serializeTo(buffer, 0, msg.type, msg.version, userBuff, msg.text, cursorX, cursorY);
	return Response{ buffer, connectedClients, msg::Type::write };
}

Response Repository::erase(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::Erase{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.eraseSize);
	int userIdx = findClient(client);
	std::scoped_lock lock{docLock};
	COORD cursorPos = doc.getCursorPos(userIdx);
	unsigned int cursorX = cursorPos.X;
	unsigned int cursorY = cursorPos.Y;
	doc.erase(userIdx, msg.eraseSize);
	logger.logInfo("user", userIdx, "erased", msg.eraseSize, "letters from document");
	buffer.clear();
	buffer.reserve(30);
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	msg::serializeTo(buffer, 0, msg.type, msg.version, userBuff, msg.eraseSize, cursorX, cursorY);
	return Response{ buffer, connectedClients, msg::Type::erase };
}

Response Repository::moveHorizontal(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::MoveHorizontal{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.side, msg.withSelect);
	int userIdx = findClient(client);
	std::scoped_lock lock{docLock};
	COORD newCursorPos = doc.getCursorPos(userIdx);
	if (msg.side == msg::MoveSide::left) {
		newCursorPos = doc.moveCursorLeft(userIdx, msg.withSelect);
		logger.logInfo("user", userIdx, "moved left");
	}
	else if (msg.side == msg::MoveSide::right) {
		newCursorPos = doc.moveCursorRight(userIdx, msg.withSelect);
		logger.logInfo("user", userIdx, "moved right");
	}
	else {
		logger.logError("Invalid MoveSide parameter in MoveHorizontal");
		return Response{ buffer, {}, msg::Type::error };
	}
	buffer.clear();
	buffer.reserve(30);
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	unsigned int cursorX = newCursorPos.X;
	unsigned int cursorY = newCursorPos.Y;
	auto anchor = doc.getCursorSelectionAnchor(userIdx);
	unsigned int anchorX = anchor.value_or(COORD{ 0, 0 }).X;
	unsigned int anchorY = anchor.value_or(COORD{ 0, 0 }).Y;
	msg::serializeTo(buffer, 0, msg.type, msg.version, userBuff, cursorX, cursorY, msg.withSelect, anchorX, anchorY);
	return Response{ buffer, connectedClients, msg::Type::moveHorizontal };
}

Response Repository::moveVertical(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::MoveVertical{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.side, msg.clientWidth, msg.withSelect);
	int userIdx = findClient(client);
	std::scoped_lock lock{docLock};
	COORD newCursorPos = doc.getCursorPos(userIdx);
	if (msg.side == msg::MoveSide::up) {
		newCursorPos = doc.moveCursorUp(userIdx, msg.clientWidth, msg.withSelect);
		logger.logInfo("user", userIdx, "moved up");
	}
	else if (msg.side == msg::MoveSide::down) {
		newCursorPos = doc.moveCursorDown(userIdx, msg.clientWidth, msg.withSelect);
		logger.logInfo("user", userIdx, "moved down");
	}
	else {
		logger.logError("Invalid MoveSide parameter in MoveHorizontal");
		return Response{ buffer, {}, msg::Type::error };
	}
	buffer.clear();
	buffer.reserve(30);
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	unsigned int cursorX = newCursorPos.X;
	unsigned int cursorY = newCursorPos.Y;
	auto anchor = doc.getCursorSelectionAnchor(userIdx);
	unsigned int anchorX = anchor.value_or(COORD{0, 0}).X;
	unsigned int anchorY = anchor.value_or(COORD{ 0, 0 }).Y;
	msg::serializeTo(buffer, 0, msg.type, msg.version, userBuff, cursorX, cursorY, msg.withSelect, anchorX, anchorY);
	return Response{ buffer, connectedClients, msg::Type::moveVertical };
}

Response Repository::moveSelectAll(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::ControlMessage{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token);
	int userIdx = findClient(client);
	std::scoped_lock lock{docLock};
	auto& data = doc.get();
	COORD newCursorPos = COORD{ static_cast<SHORT>(data[data.size() - 1].size()), static_cast<SHORT>(data.size() - 1) };
	doc.setCursorPos(userIdx, newCursorPos);
	doc.setCursorOffset(userIdx, newCursorPos.X);
	bool anchorSet = doc.setCursorAnchor(userIdx, COORD{ 0, 0 });
	logger.logInfo("user", userIdx, "selection all", (anchorSet ? "successful" : "unsuccessful"));
	buffer.clear();
	buffer.reserve(30);
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	unsigned int cursorX = newCursorPos.X;
	unsigned int cursorY = newCursorPos.Y;
	auto anchor = doc.getCursorSelectionAnchor(userIdx);
	unsigned int anchorX = anchor.value_or(COORD{ 0, 0 }).X;
	unsigned int anchorY = anchor.value_or(COORD{ 0, 0 }).Y;
	msg::serializeTo(buffer, 0, msg.type, msg.version, userBuff, cursorX, cursorY, anchorSet, anchorX, anchorY);
	return Response{ buffer, connectedClients, msg::Type::selectAll };
}

Response Repository::undoRedo(SOCKET client, msg::Buffer& buffer) {
	auto msg = msg::ControlMessage{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token);
	int userIdx = findClient(client);
	std::scoped_lock lock{docLock};

	/*auto type = action->getType();
	if (type == ActionType::noop) {
		return false;
	}
	auto text = action->getText();
	auto endPos = COORD{ 0, 0 };
	user.cursor.setPosition(action->getEndPos());
	if (type == ActionType::write) {
		endPos = erase(index, text.size(), true);
	}
	else if (type == ActionType::erase) {
		endPos = write(index, text, true);
	}
	else {
		assert(false && "Unrecognized ActionType. Aborting...");
	}
	return true*/

	if (msg.type == msg::Type::undo) {
		auto action = doc.undo(userIdx);
		auto type = action->getType();
		if (type == ActionType::noop) {
			logger.logInfo("Warning! Cannot", msg.type, "msg from user", userIdx, "with type");
			return Response{ buffer, {}, msg::Type::error };
		}
		auto text = action->getText();
		auto cursorPos = action->getEndPos();
		doc.setCursorPos(userIdx, cursorPos);
		auto endPos = COORD{ 0, 0 };
		if (type == ActionType::write) {
			endPos = doc.erase(userIdx, text.size(), true);
		}
		else if (type == ActionType::erase) {
			endPos = doc.write(userIdx, text, true);
		}
		else {
			assert(false && "Unrecognized ActionType. Aborting...");
		}
		unsigned int cursorX = cursorPos.X;
		unsigned int cursorY = cursorPos.Y;
		doc.write(userIdx, msg.text);
		logger.logInfo("user", userIdx, "wrote '" + msg.text + "' to document");


		
		return Response{ buffer, {}, msg::Type::error };
	}
	else if (msg.type == msg::Type::redo && !doc.redo(userIdx)) {
		logger.logInfo("Warning! Cannot", msg.type, "msg from user", userIdx, "with type");
		return Response{ buffer, {}, msg::Type::error };
	}
	buffer.clear();
	logger.logInfo("User", userIdx, msg.type, "action");
	auto userBuff = static_cast<msg::OneByteInt>(userIdx);
	msg::serializeTo(buffer, 0, msg.type, msg.version, userBuff);
	return Response{ buffer, connectedClients, msg.type };
}