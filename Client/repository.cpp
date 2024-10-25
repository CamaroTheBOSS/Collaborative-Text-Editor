#include "repository.h"
#include "logging.h"

Document& Repository::getDoc() {
	return doc;
}

bool Repository::processMsg(msg::Buffer& buffer) {
	msg::Type msgType;
	msg::parse(buffer, 0, msgType);

	switch (msgType) {
	case msg::Type::write:
		return write(buffer);
	case msg::Type::erase:
		return erase(buffer);
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
	parse(buffer, 1, msg.version, msg.user, msg.text);
	doc = Document(msg.text, msg.user + 1, msg.user);
	logger.logInfo("Connected to document (nCursors:", msg.user, ", text:" + msg.text + ")");
	return true;
}

bool Repository::connectNewUser(msg::Buffer& buffer) {
	logger.logInfo("Added new user to document");
	return doc.addCursor();
}

bool Repository::disconnectUser(msg::Buffer& buffer) {
	msg::DisconnectResponse msg;
	parse(buffer, 1, msg.version, msg.user);
	logger.logInfo("Disconnected user", msg.user);
	return doc.eraseCursor(msg.user);
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
	msg::MoveHorizontalResponse msg;
	parse(buffer, 1, msg.version, msg.user, msg.X, msg.Y);
	doc.setCursorPos(msg.user, COORD{ static_cast<SHORT>(msg.X), static_cast<SHORT>(msg.Y) });
	logger.logInfo("User", msg.user, "moved his cursor to", msg.X, ",", msg.Y);
	return true;
}
