#include <fstream>

#include "repository.h"
#include "pos_helpers.h"
#include "logging.h"

Document& Repository::getDoc() {
	return doc;
}

bool Repository::saveDoc() const {
	std::ofstream file(doc.getFilename(), std::ios::out);
	if (!file) {
		return false;
	}
	file << doc.getText();
	return true;
}

bool Repository::processMsg(msg::Buffer& buffer) {
	msg::Type msgType;
	msg::parse(buffer, 0, msgType);

	switch (msgType) {
	case msg::Type::write:
		return write(buffer);
	case msg::Type::erase:
		return erase(buffer);
	case msg::Type::selectAll:
	case msg::Type::moveHorizontal:
	case msg::Type::moveVertical:
		return move(buffer);
	case msg::Type::sync:
		return sync(buffer);
	case msg::Type::connect:
		return connectNewUser(buffer);
	case msg::Type::disconnect:
		return disconnectUser(buffer);
	}
	assert(false && "Unrecognized msg type. Aborting...");
	return false;
}

bool Repository::sync(msg::Buffer& buffer) {
	msg::ConnectResponse msg;
	parse(buffer, 1, msg.version, msg.user, msg.text, msg.cursorPositions);
	assert(msg.cursorPositions.size() == (msg.user + 1) * 2);
	doc = Document(msg.text, msg.user + 1, msg.user);
	for (int i = 1; i < msg.cursorPositions.size(); i += 2) {
		auto pos = COORD{ static_cast<SHORT>(msg.cursorPositions[i - 1]), static_cast<SHORT>(msg.cursorPositions[i]) };
		doc.setCursorPos(i / 2, pos);
	}
	logger.logInfo("Connected to document (nUsers:", msg.user, ", text:" + msg.text + ")");
	return true;
}

bool Repository::connectNewUser(msg::Buffer& buffer) {
	logger.logInfo("Added new user to document");
	return doc.addUser();
}

bool Repository::disconnectUser(msg::Buffer& buffer) {
	msg::DisconnectResponse msg;
	parse(buffer, 1, msg.version, msg.user);
	logger.logInfo("Disconnected user", msg.user);
	return doc.eraseUser(msg.user);
}

bool Repository::write(msg::Buffer& buffer) {
	msg::WriteResponse msg;
	parse(buffer, 1, msg.version, msg.user, msg.text);
	doc.write(msg.user, msg.text);
	logger.logInfo("User", msg.user, "wrote '" + msg.text + "' to document");
	return true;
}

bool Repository::erase(msg::Buffer& buffer) {
	msg::EraseResponse msg;
	parse(buffer, 1, msg.version, msg.user, msg.eraseSize);
	doc.erase(msg.user, msg.eraseSize);
	logger.logInfo("User", msg.user, "erased", msg.eraseSize, "from document");
	return true;
}

bool Repository::move(msg::Buffer& buffer) {
	msg::MoveResponse msg;
	parse(buffer, 1, msg.version, msg.user, msg.X, msg.Y, msg.withSelect, msg.anchorX, msg.anchorY);
	doc.moveTo(msg.user, makeCoord(msg.X, msg.Y), makeCoord(msg.anchorX, msg.anchorY), msg.withSelect);
	logger.logInfo("User", msg.user, "moved his cursor to", msg.X, ",", msg.Y);
	return true;
}
