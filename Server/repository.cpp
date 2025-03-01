#include <thread>

#include "serializer.h"
#include "deserializer.h"
#include "repository.h"
#include "logging.h"
#include "engine.h"

namespace server {

	Response Repository::process(SOCKET client, msg::Buffer& buffer) {
		msg::Type type;
		msg::OneByteInt version;
		msg::parse(buffer, 0, type, version);
		switch (type) {
		case msg::Type::create:
			return createDoc(client, buffer);
		case msg::Type::join:
			return loadDoc(client, buffer);
		case msg::Type::masterClose:
			return masterClose(buffer);
		}
		auto doc = findDoc(client);
		if (doc == nullptr) {
			logger.logError("Document for client", client, "not found");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		ArgPack argPack{ client, buffer, doc };
		switch (type) {
		case msg::Type::disconnect:
			return disconnectUserFromDoc(argPack);
		case msg::Type::write:
			return write(argPack);
		case msg::Type::erase:
			return erase(argPack);
		case msg::Type::moveHorizontal:
			return moveHorizontal(argPack);
		case msg::Type::moveVertical:
			return moveVertical(argPack);
		case msg::Type::moveTo:
			return moveTo(argPack);
		case msg::Type::selectAll:
			return moveSelectAll(argPack);
		case msg::Type::undo:
		case msg::Type::redo:
			return undoRedo(argPack);
		case msg::Type::replace:
			return replace(argPack);
		}
		assert(false && "Unrecognized msg type. Aborting...");
		return Response{ std::move(buffer), {}, msg::Type::error };
	}

	std::string Repository::getLastAddedAcCode() {
		std::string acCode = lastAddedAcCode;
		lastAddedAcCode.clear();
		return acCode;
	}
	std::string Repository::getLastDeletedAcCode() {
		std::string acCode = lastDeletedAcCode;
		lastDeletedAcCode.clear();
		return acCode;
	}

	ServerSiteDocument* Repository::findDoc(SOCKET client) {
		auto acCodeIt = clientToAcCodeMap.find(client);
		if (acCodeIt == clientToAcCodeMap.cend()) {
			return nullptr;
		}
		auto docIt = acCodeToDocMap.find(acCodeIt->second);
		if (docIt == acCodeToDocMap.cend()) {
			return {};
		}
		return &docIt->second;
	}

	Response Repository::masterClose(msg::Buffer& buffer) const {
		logger.logDebug("Thread", std::this_thread::get_id(), "got msg from master to close itself!");
		return Response{ std::move(buffer), {}, msg::Type::masterClose };
	}

	Response Repository::createDoc(const SOCKET client, msg::Buffer& buffer) {
		auto msg = Deserializer::parseConnectCreateDoc(buffer);
		auto acCode = random::Engine::get().getRandomString(6);
		clientToAcCodeMap.emplace(msg.socket, acCode);
		auto docIt = acCodeToDocMap.emplace(acCode, ServerSiteDocument("", 1, 0, msg.filename));
		docIt.first->second.addClient(msg.socket);
		logger.logDebug("User", client, "created new document!");
		auto newBuffer = Serializer::makeConnectResponse(msg.type, docIt.first->second, msg.version, 0, acCode);
		lastAddedAcCode = acCode;
		return Response{ std::move(newBuffer), docIt.first->second.getConnectedClients(), msg::Type::create };
	}

	Response Repository::loadDoc(const SOCKET client, msg::Buffer& buffer) {
		auto msg = Deserializer::parseConnectJoinDoc(buffer);
		auto docIt = acCodeToDocMap.find(msg.acCode);
		if (docIt == acCodeToDocMap.cend()) {
			std::string errMsg = "Incorrect access code!";
			auto newBuffer = Serializer::makeConnectResponseWithError(msg.type, errMsg, 1);
			return Response{ std::move(newBuffer), { msg.socket }, msg::Type::join };
		}
		clientToAcCodeMap.emplace(msg.socket, msg.acCode);
		docIt->second.addUser();
		docIt->second.addClient(msg.socket);
		int userIdx = docIt->second.getCursorNum() - 1;
		auto newBuffer = Serializer::makeConnectResponse(msg.type, docIt->second, msg.version, userIdx, msg.acCode);
		logger.logDebug("User", client, "connected to document");
		return Response{ std::move(newBuffer), docIt->second.getConnectedClients(), msg::Type::join };
	}

	Response Repository::disconnectUserFromDoc(const ArgPack& argPack) {
		auto msg = Deserializer::parseDisconnect(argPack.buffer);
		auto& doc = *argPack.doc;
		int userIdx = doc.findUser(argPack.client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
		doc.eraseUser(userIdx);
		doc.eraseClient(argPack.client);
		eraseFromMap(doc, argPack.client);
		auto newBuffer = Serializer::makeDisconnectResponse(userIdx, msg);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::disconnect };
	}

	void Repository::eraseFromMap(const ServerSiteDocument& doc, const SOCKET client) {
		auto clientToAcCodeIt = clientToAcCodeMap.find(client);
		std::string erasedAcCode;
		if (clientToAcCodeIt != clientToAcCodeMap.cend()) {
			erasedAcCode = clientToAcCodeIt->second;
			clientToAcCodeMap.erase(clientToAcCodeIt);
		}
		if (doc.getCursorNum() == 0) {
			auto acCodeToDocIt = acCodeToDocMap.find(erasedAcCode);
			if (acCodeToDocIt != acCodeToDocMap.cend()) {
				lastDeletedAcCode = erasedAcCode;
				acCodeToDocMap.erase(acCodeToDocIt);
			}
		}
	}

	Response Repository::write(const ArgPack& argPack) {
		auto msg = Deserializer::parseWrite(argPack.buffer);
		auto& doc = *argPack.doc;
		int userIdx = doc.findUser(argPack.client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
		COORD startPos = doc.getCursorPos(userIdx);
		doc.write(userIdx, msg.text);
		logger.logInfo("User", userIdx, "wrote", msg.text.size(), "letters");
		auto newBuffer = Serializer::makeWriteResponse(startPos, userIdx, msg);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::write };
	}

	Response Repository::erase(const ArgPack& argPack) {
		auto msg = Deserializer::parseErase(argPack.buffer);
		auto& doc = *argPack.doc;
		int userIdx = doc.findUser(argPack.client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
		COORD startPos = doc.getCursorPos(userIdx);
		doc.erase(userIdx, msg.eraseSize);
		logger.logInfo("User", userIdx, "erased", msg.eraseSize, "letters from document");
		auto newBuffer = Serializer::makeEraseResponse(startPos, userIdx, msg);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::erase };
	}

	Response Repository::moveHorizontal(const ArgPack& argPack) {
		auto msg = Deserializer::parseMoveHorizontal(argPack.buffer);
		auto& doc = *argPack.doc;
		int userIdx = doc.findUser(argPack.client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
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
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
		auto newBuffer = Serializer::makeMoveResponse(*argPack.doc, userIdx, msg);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::moveHorizontal };
	}

	Response Repository::moveVertical(const ArgPack& argPack) {
		auto msg = Deserializer::parseMoveVertical(argPack.buffer);
		auto& doc = *argPack.doc;
		int userIdx = doc.findUser(argPack.client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
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
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
		auto newBuffer = Serializer::makeMoveResponse(*argPack.doc, userIdx, msg);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::moveVertical };
	}

	Response Repository::moveTo(const ArgPack& argPack) {
		auto msg = Deserializer::parseMoveTo(argPack.buffer);
		auto& doc = *argPack.doc;
		int userIdx = doc.findUser(argPack.client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
		doc.setCursorPos(userIdx, COORD{ (SHORT)msg.X, (SHORT)msg.Y });
		auto newBuffer = Serializer::makeMoveResponse(*argPack.doc, userIdx, msg);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::moveTo };
	}

	Response Repository::moveSelectAll(const ArgPack& argPack) {
		auto msg = Deserializer::parseMoveSelectAll(argPack.buffer);
		auto& doc = *argPack.doc;
		int userIdx = doc.findUser(argPack.client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
		doc.setCursorPos(userIdx, doc.getEndPos());
		doc.setCursorAnchor(userIdx, COORD{ 0, 0 });
		logger.logInfo("User", userIdx, "selected all");
		auto newBuffer = Serializer::makeMoveResponse(*argPack.doc, userIdx, msg);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::selectAll };
	}

	Response Repository::undoRedo(const ArgPack& argPack) {
		auto msg = Deserializer::parseControlMessage(argPack.buffer);
		auto& doc = *argPack.doc;
		int userIdx = doc.findUser(argPack.client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
		auto undoReturn = msg.type == msg::Type::undo ? doc.undo(userIdx) : doc.redo(userIdx);
		if (undoReturn.type == ActionType::write) {
			msg::Write newMsg{ msg::Type::write, msg.version, "", undoReturn.text };
			auto newBuffer = Serializer::makeWriteResponse(undoReturn.startPos, userIdx, newMsg);
			return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::write };
		}
		else if (undoReturn.type == ActionType::erase) {
			msg::Erase newMsg{ msg::Type::erase, msg.version, "", undoReturn.text.size() };
			auto newBuffer = Serializer::makeEraseResponse(undoReturn.startPos, userIdx, newMsg);
			return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::erase };
		}
		logger.logDebug(msg.type, "returned noop action. Nothing changed.");
		return Response{ std::move(argPack.buffer), {}, msg::Type::error };
	}

	Response Repository::replace(const ArgPack& argPack) {
		auto msg = Deserializer::parseReplaceMessage(argPack.buffer);
		auto& doc = *argPack.doc;
		int userIdx = doc.findUser(argPack.client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
		for (auto segment = msg.segments.rbegin(); segment != msg.segments.rend(); segment++) {
			if (!doc.setCursorPos(userIdx, segment->first) || !doc.setCursorAnchor(userIdx, segment->second)) {
				logger.logError("Replace failed with one segment");
				continue;
			}
			doc.erase(userIdx, 1);
			doc.write(userIdx, msg.text);
		}
		auto newBuffer = Serializer::makeReplaceResponse(userIdx, msg);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::replace };
	}

}