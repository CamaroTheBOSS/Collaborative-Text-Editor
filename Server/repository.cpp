#include <thread>

#include "serializer.h"
#include "deserializer.h"
#include "repository.h"
#include "logging.h"

namespace server {

	Response Repository::process(SOCKET client, msg::Buffer& buffer) {
		msg::Type type;
		msg::OneByteInt version;
		msg::parse(buffer, 0, type, version);
		switch (type) {
		case msg::Type::masterNotification:
			return masterNotification(client, buffer);
		case msg::Type::masterClose:
			return masterClose(client, buffer);
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
		case msg::Type::replace:
			return replace(client, buffer);
		}
		assert(false && "Unrecognized msg type. Aborting...");
		return Response{ std::move(buffer), {}, msg::Type::error };
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
		auto msg = Deserializer::parseConnect(buffer);
		std::scoped_lock lock{connectedClientsLock, docLock};
		msg::OneByteInt userIdx = connectedClients.size();
		connectedClients.push_back(client);
		doc.addUser();
		auto newBuffer = Serializer::makeConnectResponse(doc, userIdx, msg);
		logger.logDebug("Thread", std::this_thread::get_id(), "connected new user to document!");
		return Response{ std::move(newBuffer), { connectedClients }, msg::Type::sync };
	}

	Response Repository::disconnectUserFromDoc(SOCKET client, msg::Buffer& buffer) {
		auto msg = Deserializer::parseDisconnect(buffer);
		int userIdx = findClient(client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		std::scoped_lock lock{connectedClientsLock, docLock};
		connectedClients.erase(connectedClients.cbegin() + userIdx);
		doc.eraseUser(userIdx);
		auto newBuffer = Serializer::makeDisconnectResponse(userIdx, msg);
		return Response{ std::move(newBuffer), connectedClients, msg::Type::disconnect };
	}

	Response Repository::masterNotification(SOCKET client, msg::Buffer& buffer) const {
		logger.logDebug("Thread", std::this_thread::get_id(), "got new connection!");
		return Response{ std::move(buffer), {}, msg::Type::masterNotification };
	}

	Response Repository::masterClose(SOCKET client, msg::Buffer& buffer) const {
		logger.logDebug("Thread", std::this_thread::get_id(), "got msg from master to close itself!");
		return Response{ std::move(buffer), {}, msg::Type::masterClose };
	}

	Response Repository::write(SOCKET client, msg::Buffer& buffer) {
		auto msg = Deserializer::parseWrite(buffer);
		int userIdx = findClient(client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		std::scoped_lock lock{docLock};
		COORD startPos = doc.getCursorPos(userIdx);
		doc.write(userIdx, msg.text);
		logger.logInfo("User", userIdx, "wrote", msg.text.size(), "letters");
		auto newBuffer = Serializer::makeWriteResponse(startPos, userIdx, msg);
		return Response{ std::move(newBuffer), connectedClients, msg::Type::write };
	}

	Response Repository::erase(SOCKET client, msg::Buffer& buffer) {
		auto msg = Deserializer::parseErase(buffer);
		int userIdx = findClient(client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		std::scoped_lock lock{docLock};
		COORD startPos = doc.getCursorPos(userIdx);
		doc.erase(userIdx, msg.eraseSize);
		logger.logInfo("User", userIdx, "erased", msg.eraseSize, "letters from document");
		auto newBuffer = Serializer::makeEraseResponse(startPos, userIdx, msg);
		return Response{ std::move(newBuffer), connectedClients, msg::Type::erase };
	}

	Response Repository::moveHorizontal(SOCKET client, msg::Buffer& buffer) {
		auto msg = Deserializer::parseMoveHorizontal(buffer);
		int userIdx = findClient(client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		std::scoped_lock lock{docLock};
		if (msg.side == msg::MoveSide::left) {
			doc.moveCursorLeft(userIdx, msg.withSelect);
			logger.logInfo("User", userIdx, "moved left");
		}
		else if (msg.side == msg::MoveSide::right) {
			doc.moveCursorRight(userIdx, msg.withSelect);
			logger.logInfo("User", userIdx, "moved right");
		}
		else {
			logger.logError("Invalid MoveSide parameter in MoveHorizontal");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		auto newBuffer = Serializer::makeMoveResponse(doc, userIdx, msg);
		return Response{ std::move(newBuffer), connectedClients, msg::Type::moveHorizontal };
	}

	Response Repository::moveVertical(SOCKET client, msg::Buffer& buffer) {
		auto msg = Deserializer::parseMoveVertical(buffer);
		int userIdx = findClient(client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		std::scoped_lock lock{docLock};
		if (msg.side == msg::MoveSide::up) {
			doc.moveCursorUp(userIdx, msg.clientWidth, msg.withSelect);
			logger.logInfo("User", userIdx, "moved up");
		}
		else if (msg.side == msg::MoveSide::down) {
			doc.moveCursorDown(userIdx, msg.clientWidth, msg.withSelect);
			logger.logInfo("User", userIdx, "moved down");
		}
		else {
			logger.logError("Invalid MoveSide parameter in MoveHorizontal");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		auto newBuffer = Serializer::makeMoveResponse(doc, userIdx, msg);
		return Response{ std::move(newBuffer), connectedClients, msg::Type::moveVertical };
	}

	Response Repository::moveSelectAll(SOCKET client, msg::Buffer& buffer) {
		auto msg = Deserializer::parseMoveSelectAll(buffer);
		int userIdx = findClient(client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		std::scoped_lock lock{docLock};
		doc.setCursorPos(userIdx, doc.getEndPos());
		doc.setCursorAnchor(userIdx, COORD{ 0, 0 });
		logger.logInfo("User", userIdx, "selected all");
		auto newBuffer = Serializer::makeMoveResponse(doc, userIdx, msg);
		return Response{ std::move(newBuffer), connectedClients, msg::Type::selectAll };
	}

	Response Repository::undoRedo(SOCKET client, msg::Buffer& buffer) {
		auto msg = Deserializer::parseControlMessage(buffer);
		int userIdx = findClient(client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		std::scoped_lock lock{docLock};
		auto undoReturn = msg.type == msg::Type::undo ? doc.undo(userIdx) : doc.redo(userIdx);
		if (undoReturn.type == ActionType::write) {
			msg::Write newMsg{ msg::Type::write, msg.version, "", undoReturn.text };
			auto newBuffer = Serializer::makeWriteResponse(undoReturn.startPos, userIdx, newMsg);
			return Response{ std::move(newBuffer), connectedClients, msg::Type::write };
		}
		else if (undoReturn.type == ActionType::erase) {
			msg::Erase newMsg{ msg::Type::erase, msg.version, "", undoReturn.text.size() };
			auto newBuffer = Serializer::makeEraseResponse(undoReturn.startPos, userIdx, newMsg);
			return Response{ std::move(newBuffer), connectedClients, msg::Type::erase };
		}
		logger.logDebug(msg.type, "returned noop action. Nothing changed.");
		return Response{ std::move(buffer), {}, msg::Type::error };
	}

	Response Repository::replace(SOCKET client, msg::Buffer& buffer) {
		auto msg = Deserializer::parseReplaceMessage(buffer);
		int userIdx = findClient(client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		std::scoped_lock lock{docLock};
		for (auto segment = msg.segments.rbegin(); segment != msg.segments.rend(); segment++) {
			if (!doc.setCursorPos(userIdx, segment->first) || !doc.setCursorAnchor(userIdx, segment->second)) {
				logger.logError("Replace failed with one segment");
				continue;
			}
			doc.erase(userIdx, 1);
			doc.write(userIdx, msg.text);
		}
		auto newBuffer = Serializer::makeReplaceResponse(userIdx, msg);
		return Response{ std::move(newBuffer), connectedClients, msg::Type::replace };
	}

}