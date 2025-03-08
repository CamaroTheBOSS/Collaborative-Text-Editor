#include <thread>

#include "serializer.h"
#include "deserializer.h"
#include "repository.h"
#include "logging.h"
#include "engine.h"

namespace server {

	Repository::Repository(server::Authenticator* auth) :
		auth(auth) {}
	
	Repository::Repository(Repository&& other) :
		clientToUserData(std::move(other.clientToUserData)),
		acCodeToDocMap(std::move(other.acCodeToDocMap)),
		auth(other.auth),
		savingDocInterval(std::move(other.savingDocInterval)),
		db(std::move(other.db)),
		acCodesLock(),
		acCodeSet(std::move(other.acCodeSet)),
		userFileCombinedLock(),
		userFileCombinedSet(std::move(other.userFileCombinedSet)) {}

	Repository& Repository::operator=(Repository&& other) {
		clientToUserData = std::move(other.clientToUserData);
		acCodeToDocMap = std::move(other.acCodeToDocMap);
		auth = auth;
		savingDocInterval = std::move(other.savingDocInterval);
		db = std::move(other.db);
		acCodeSet = std::move(other.acCodeSet);
		userFileCombinedSet = std::move(other.userFileCombinedSet);
		return *this;
	}

	Response Repository::process(SOCKET client, msg::Buffer& buffer, bool authenticateUser) {
		msg::Type type;
		msg::OneByteInt version;
		int pos = msg::parse(buffer, 0, type, version);
		// Messages from master, doesn't require authentication
		switch (type) {
		case msg::Type::create:
			return createDoc(buffer);
		case msg::Type::load:
			return loadDoc(buffer);
		case msg::Type::join:
			return joinDoc(buffer);
		case msg::Type::masterClose:
			return masterClose(buffer);
		}

		std::string authToken;
		msg::parse(buffer, pos, authToken);
		if (authenticateUser) {
			auto it = clientToUserData.find(client);
			if (it == clientToUserData.cend() || it->second.authToken != authToken) {
				logger.logError("Cannot authenticate user", client);
				return Response{ std::move(buffer), {}, msg::Type::error };
			}
		}

		auto doc = findDoc(client);
		if (doc == nullptr) {
			if (type == msg::Type::disconnect) {
				auth->clearUser(client);
			}
			logger.logDebug("Document for client", client, "not found");
			return Response{ std::move(buffer), {}, msg::Type::error };
		}
		ArgPack argPack{ client, buffer, doc };
		auto response = processImpl(type, argPack);
		if (type != msg::Type::disconnect && std::chrono::system_clock::now() > doc->getLastSaveTimestamp() + savingDocInterval) {
			saveDocInDb(*doc);
		}
		return response;
	}

	Response Repository::processImpl(const msg::Type type, const ArgPack& argPack) {
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
		return Response{ std::move(argPack.buffer), {}, msg::Type::error };
	}

	ServerSiteDocument* Repository::findDoc(SOCKET client) {
		auto userData = clientToUserData.find(client);
		if (userData == clientToUserData.cend()) {
			return nullptr;
		}
		auto docIt = acCodeToDocMap.find(userData->second.acCode);
		if (docIt == acCodeToDocMap.cend()) {
			return {};
		}
		return &docIt->second;
	}

	bool Repository::acCodeExists(const std::string& acCode) {
		std::lock_guard lock{acCodesLock};
		return std::find(acCodeSet.cbegin(), acCodeSet.cend(), acCode) != acCodeSet.cend();
	}
	bool Repository::userFileExists(const std::string& username, const std::string& filename) {
		std::lock_guard lock{userFileCombinedLock};
		auto key = username + "-" + filename;
		return std::find(userFileCombinedSet.cbegin(), userFileCombinedSet.cend(), key) != userFileCombinedSet.cend();
	}

	Response Repository::masterClose(msg::Buffer& buffer) const {
		logger.logDebug("Thread", std::this_thread::get_id(), "got msg from master to close itself!");
		return Response{ std::move(buffer), {}, msg::Type::masterClose };
	}

	Response Repository::createDoc(msg::Buffer& buffer) {
		auto msg = Deserializer::parseConnectCreateDoc(buffer);
		auto id = random::Engine::get().getRandomString(12);
		auto userAuthData = auth->getUserData(msg.socket);
		assert(!userAuthData.authToken.empty());
		DBDocument dbDoc(id, msg.filename, { auth->getUserData(msg.socket).username });
		if (!db.addDocAndLink(dbDoc)) {
			auto newBuffer = Serializer::makeConnectResponseWithError(msg.type, db.getLastError(), 1);
			return Response{ std::move(newBuffer), { msg.socket }, msg::Type::create };
		}
		auto session = createNewSession(userAuthData.username, ServerSiteDocument("", 0, 0, id, msg.filename));
		addClientToSession(msg.socket, userAuthData, session);
		auto& [acCode, doc] = *session;
		auto newBuffer = Serializer::makeConnectResponse(msg.type, doc, msg.version, 0, acCode);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::create };
	}

	Response Repository::loadDoc(msg::Buffer& buffer) {
		auto msg = Deserializer::parseConnectCreateDoc(buffer);
		auto userAuthData = auth->getUserData(msg.socket);
		assert(!userAuthData.authToken.empty());
		auto docIt = db.loadDoc(userAuthData.username, msg.filename);
		if (!docIt) {
			auto newBuffer = Serializer::makeConnectResponseWithError(msg.type, db.getLastError(), 1);
			return Response{ std::move(newBuffer), { msg.socket }, msg::Type::load };
		}
		auto session = getSessionWithDocId(docIt.value().getId());
		if (session == acCodeToDocMap.end()) {
			session = createNewSession(userAuthData.username, docIt.value());
		}
		addClientToSession(msg.socket, userAuthData, session);
		auto& [acCode, doc] = *session;
		auto userIdx = doc.findUser(msg.socket);
		auto newBuffer = Serializer::makeConnectResponse(msg.type, doc, msg.version, userIdx, acCode);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::load };
	}

	Response Repository::joinDoc(msg::Buffer& buffer) {
		auto msg = Deserializer::parseConnectJoinDoc(buffer);
		auto session = getSessionWithAcCode(msg.acCode);
		if (session == acCodeToDocMap.end()) {
			std::string errMsg = "Incorrect access code!";
			auto newBuffer = Serializer::makeConnectResponseWithError(msg.type, errMsg, 1);
			return Response{ std::move(newBuffer), { msg.socket }, msg::Type::join };
		}
		auto& [acCode, doc] = *session;
		auto userAuthData = auth->getUserData(msg.socket);
		assert(!userAuthData.authToken.empty());
		DBUser userdb(userAuthData.username, "", {});
		DBDocument docdb(doc.getId(), "", {});
		if (!db.linkUserAndDoc(userdb, docdb)) {
			auto newBuffer = Serializer::makeConnectResponseWithError(msg.type, db.getLastError(), 1);
			return Response{ std::move(newBuffer), { msg.socket }, msg::Type::join };
		}
		addClientToSession(msg.socket, userAuthData, session);
		int userIdx = doc.getCursorNum() - 1;
		auto newBuffer = Serializer::makeConnectResponse(msg.type, doc, msg.version, userIdx, acCode);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::join };
	}

	Response Repository::disconnectUserFromDoc(const ArgPack& argPack) {
		auto msg = Deserializer::parseDisconnect(argPack.buffer);
		auto& doc = *argPack.doc;
		int userIdx = doc.findUser(argPack.client);
		if (userIdx < 0) {
			logger.logDebug(msg.type, "command failed. User not found error");
			return Response{ std::move(argPack.buffer), {}, msg::Type::error };
		}
		saveDocInDb(doc);
		eraseClientFromSession(doc, argPack.client);
		auto newBuffer = Serializer::makeDisconnectResponse(userIdx, msg);
		return Response{ std::move(newBuffer), doc.getConnectedClients(), msg::Type::disconnect };
	}

	void Repository::eraseClientFromSession(ServerSiteDocument& doc, const SOCKET client) {
		int userIdx = doc.findUser(client);
		doc.eraseUser(userIdx);
		doc.eraseClient(client);
		auto userDataIt = clientToUserData.find(client);
		std::string erasedAcCode, erasedUsername;
		if (userDataIt != clientToUserData.cend()) {
			erasedAcCode = std::move(userDataIt->second.acCode);
			erasedUsername = std::move(userDataIt->second.username);
			clientToUserData.erase(userDataIt);
			std::lock_guard lock{userFileCombinedLock};
			userFileCombinedSet.erase(erasedUsername + "-" + doc.getFilename());
		}
		auth->clearUser(client);
		if (doc.getCursorNum() == 0) {
			deleteSession(erasedUsername, erasedAcCode, doc);
		}
	}

	void Repository::deleteSession(const std::string& username, const std::string& acCode, ServerSiteDocument& doc) {
		std::scoped_lock lock{acCodesLock, userFileCombinedLock};
		acCodeSet.erase(acCode);
		userFileCombinedSet.erase(username + "-" + doc.getFilename());
		auto acCodeToDocIt = acCodeToDocMap.find(acCode);
		if (acCodeToDocIt != acCodeToDocMap.cend()) {
			acCodeToDocMap.erase(acCodeToDocIt);
		}
	}

	bool Repository::saveDocInDb(const ServerSiteDocument& doc) {
		return db.saveDoc(doc.getId(), doc.getText());
	}

	SessionIt Repository::getSessionWithDocId(const std::string& id) {
		for (auto it = acCodeToDocMap.begin(); it != acCodeToDocMap.end(); it++) {
			if (it->second.getId() == id) {
				return it;
			}
		}
		return acCodeToDocMap.end();
	}

	SessionIt Repository::getSessionWithAcCode(const std::string& acCode) {
		return acCodeToDocMap.find(acCode);
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

	bool Repository::addClientToSession(const SOCKET client, Authenticator::UserData& userAuthData, SessionIt session) {
		auto& [acCode, doc] = *session;
		doc.addClient(client);
		doc.addUser();
		std::lock_guard lock{userFileCombinedLock};
		userFileCombinedSet.insert(userAuthData.username + "-" + doc.getFilename());
		clientToUserData.emplace(client, ClientUserData{ std::move(acCode), std::move(userAuthData.username), std::move(userAuthData.authToken) });
		logger.logDebug("User", client, "added to session (docId", doc.getId() + ")");
		return true;
	}
}