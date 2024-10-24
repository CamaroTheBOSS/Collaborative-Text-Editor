#include "repository.h"
#include "logging.h"

Document& Repository::getDoc() {
	return doc;
}

void Repository::processMsg(msg::Buffer& buffer) {
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
	}
}

void Repository::write(msg::Buffer& buffer) {
	msg::WriteResponse msg;
	parse(buffer, 1, msg.version, msg.user, msg.text);
	doc.write(msg.user, msg.text);
	logger.logInfo("User ", msg.user, " wrote '" + msg.text + "' to document");
}

void Repository::erase(msg::Buffer& buffer) {
	msg::EraseResponse msg;
	parse(buffer, 1, msg.version, msg.user, msg.eraseSize);
	doc.erase(msg.user, msg.eraseSize);
	logger.logInfo("User ", msg.user, " erased ", msg.eraseSize, " from document");
}

void Repository::move(msg::Buffer& buffer) {
	msg::MoveHorizontalResponse msg;
	parse(buffer, 1, msg.version, msg.user, msg.X, msg.Y);
	doc.setCursorPos(msg.user, COORD{ static_cast<SHORT>(msg.X), static_cast<SHORT>(msg.Y) });
	logger.logInfo("User ", msg.user, " moved his cursor to ", msg.X, ",", msg.Y);
}
