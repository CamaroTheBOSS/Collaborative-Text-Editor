#include "repository.h"
#include "pos_helpers.h"
#include "logging.h"

namespace client {
	bool Repository::processMsg(ClientSiteDocument& doc, msg::Buffer& buffer) {
		msg::Type msgType;
		msg::parse(buffer, 0, msgType);

		switch (msgType) {
		case msg::Type::write:
			return write(doc, buffer);
		case msg::Type::erase:
			return erase(doc, buffer);
		case msg::Type::selectAll:
		case msg::Type::moveHorizontal:
		case msg::Type::moveVertical:
		case msg::Type::moveTo:
			return move(doc, buffer);
		case msg::Type::create:
		case msg::Type::join:
			return sync(doc, buffer);
		case msg::Type::connect:
			return connectNewUser(doc, buffer);
		case msg::Type::disconnect:
			return disconnectUser(doc, buffer);
		case msg::Type::replace:
			return replace(doc, buffer);
		case msg::Type::login:
			return login(doc, buffer);
		case msg::Type::logout:
			return logout(doc, buffer);
		case msg::Type::registration:
			return registered(doc, buffer);
		}
		assert(false && "Unrecognized msg type. Aborting...");
		return false;
	}

	bool Repository::sync(ClientSiteDocument& doc, msg::Buffer& buffer) {
		msg::ConnectResponse msg;
		parse(buffer, 0, msg.type, msg.version, msg.user, lastError, acCode, msg.text, msg.cursorPositions);
		if (!lastError.empty()) {
			logger.logDebug(lastError);
			return false;
		}
		assert(msg.cursorPositions.size() == (msg.user + 1) * 2);
		doc = ClientSiteDocument(msg.text, msg.user + 1, msg.user);
		for (int i = 1; i < msg.cursorPositions.size(); i += 2) {
			auto pos = COORD{ static_cast<SHORT>(msg.cursorPositions[i - 1]), static_cast<SHORT>(msg.cursorPositions[i]) };
			doc.setCursorPos(i / 2, pos);
		}
		logger.logInfo("Connected to document (nUsers:", msg.user, ", text:" + msg.text + ")");
		return true;
	}

	bool Repository::connectNewUser(ClientSiteDocument& doc, msg::Buffer& buffer) {
		logger.logInfo("Added new user to document");
		return doc.addUser();
	}

	bool Repository::disconnectUser(ClientSiteDocument& doc, msg::Buffer& buffer) {
		msg::DisconnectResponse msg;
		parse(buffer, 1, msg.version, msg.user);
		logger.logInfo("Disconnected user", msg.user);
		return doc.eraseUser(msg.user);
	}

	bool Repository::replace(ClientSiteDocument& doc, msg::Buffer& buffer) {
		msg::ReplaceResponse msg;
		parse(buffer, 1, msg.version, msg.user, msg.text, msg.segments);
		int myUser = doc.getMyCursor();
		if (msg.user == myUser) {
			doc.resetSegments();
		}
		for (auto segment = msg.segments.rbegin(); segment != msg.segments.rend(); segment++) {
			if (!doc.setCursorPos(msg.user, segment->first) || !doc.setCursorAnchor(msg.user, segment->second)) {
				logger.logError("Replace failed with one segment");
				continue;
			}
			COORD start = doc.erase(msg.user, 1);
			COORD end = doc.write(msg.user, msg.text);
			if (msg.user == myUser) {
				doc.insertSegment(start, end, 0);
			}
		}
		return true;
	}

	bool Repository::write(ClientSiteDocument& doc, msg::Buffer& buffer) {
		msg::WriteResponse msg;
		parse(buffer, 1, msg.version, msg.user, msg.text, msg.X, msg.Y);
		doc.setCursorPos(msg.user, makeCoord(msg.X, msg.Y));
		doc.write(msg.user, msg.text);
		logger.logInfo("User", msg.user, "wrote '" + msg.text + "' to document");
		return true;
	}

	bool Repository::erase(ClientSiteDocument& doc, msg::Buffer& buffer) {
		msg::EraseResponse msg;
		parse(buffer, 1, msg.version, msg.user, msg.eraseSize, msg.X, msg.Y);
		doc.setCursorPos(msg.user, makeCoord(msg.X, msg.Y));
		doc.erase(msg.user, msg.eraseSize);
		logger.logInfo("User", msg.user, "erased", msg.eraseSize, "from document");
		return true;
	}

	bool Repository::move(ClientSiteDocument& doc, msg::Buffer& buffer) {
		msg::MoveResponse msg;
		parse(buffer, 1, msg.version, msg.user, msg.X, msg.Y, msg.withSelect, msg.anchorX, msg.anchorY);
		doc.moveTo(msg.user, makeCoord(msg.X, msg.Y), makeCoord(msg.anchorX, msg.anchorY), msg.withSelect);
		logger.logInfo("User", msg.user, "moved his cursor to", msg.X, ",", msg.Y);
		return true;
	}

	bool Repository::login(ClientSiteDocument& doc, msg::Buffer& buffer) {
		msg::LoginResponse msg;
		parse(buffer, 1, msg.version, msg.errMsg, msg.authToken);
		if (!msg.errMsg.empty()) {
			logger.logInfo("Error during login:", msg.errMsg);
			lastError = msg.errMsg;
			return false;
		}
		authToken = msg.authToken;
		return true;
	}

	bool Repository::logout(ClientSiteDocument& doc, msg::Buffer& buffer) {
		msg::AckResponse msg;
		parse(buffer, 1, msg.version);
		return true;
	}

	bool Repository::registered(ClientSiteDocument& doc, msg::Buffer& buffer) {
		msg::RegisterResponse msg;
		parse(buffer, 1, msg.version, msg.errMsg);
		if (!msg.errMsg.empty()) {
			logger.logInfo("Error during login:", msg.errMsg);
			lastError = msg.errMsg;
			return false;
		}
		return true;
	}

	std::string Repository::getAcCode() const {
		return acCode;
	}
	std::string Repository::getAuthToken() const {
		return authToken;
	}
	void Repository::cleanAcCode() {
		acCode.clear();
	}
	void Repository::cleanAuthToken() {
		return authToken.clear();
	}
	std::string Repository::getLastError() {
		std::string err = lastError;
		lastError.clear();
		return err;
	}

}
