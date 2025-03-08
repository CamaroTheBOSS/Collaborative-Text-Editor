#include "authenticator.h"
#include "deserializer.h"
#include "serializer.h"
#include "engine.h"
#include "logging.h"

namespace server {
	Response Authenticator::process(SOCKET client, msg::Buffer& buffer) {
		msg::Type type;
		msg::OneByteInt version;
		msg::parse(buffer, 0, type, version);
		ArgPack args{ client, buffer };
		switch (type) {
		case msg::Type::login:
			return loginUser(args);
		case msg::Type::logout:
			return logoutUser(args);
		case msg::Type::registration:
			return registerUser(args);
		case msg::Type::getDocNames:
			return getDocNames(args);
		case msg::Type::delDoc:
			return delDoc(args);
		}
		assert(false && "Unrecognized msg type. Aborting...");
		return Response{ buffer, {}, msg::Type::error };
	}
	
	Response Authenticator::loginUser(const ArgPack& args) {
		auto msg = Deserializer::parseLogin(args.buffer);
		auto userFromDbOpt = db.getUserWithUsername(msg.login);
		std::string errMsg;
		if (!userFromDbOpt) {
			auto buffer = Serializer::makeLoginResponse(msg.version, "", db.getLastError());
			return Response{ buffer, {args.client}, msg.type };
		}
		auto& dbUser = userFromDbOpt.value();
		std::string authToken = getAuthToken(args.client);
		if (checkIfUserIsActive(dbUser.username) || !authToken.empty()) {
			errMsg = "Session for this user already exists!";
		}
		else if (errMsg.empty()) {
			if (dbUser.password == msg.password) {
				authToken = random::Engine::get().getRandomString(16);
				addUser(args.client, authToken, dbUser.username);
			}
			else {
				errMsg = "Incorrect password!";
			}
		}
		auto buffer = Serializer::makeLoginResponse(msg.version, authToken, errMsg);
		return Response{ buffer, {args.client}, msg.type };
	}

	Response Authenticator::logoutUser(const ArgPack& args) {
		auto msg = Deserializer::parseAck(args.buffer);
		auto buffer = Serializer::makeAckResponse(msg.type, msg.version);
		clearUser(args.client);
		return Response{ buffer, {}, msg.type };
	}

	Response Authenticator::registerUser(const ArgPack& args) {
		auto msg = Deserializer::parseRegister(args.buffer);
		DBUser dbUser;
		dbUser.username = std::move(msg.login);
		dbUser.password = std::move(msg.password);
		auto success = db.addUser(dbUser);
		auto buffer = Serializer::makeRegisterResponse(msg.version, success ? "" : db.getLastError());
		return Response{ buffer, {args.client}, msg.type };
	}

	Response Authenticator::getDocNames(const ArgPack& args) {
		auto msg = Deserializer::parseControlMessage(args.buffer);
		auto userData = getUserData(args.client);
		if (userData.authToken != msg.authToken) {
			logger.logError("Cannot authenticate user", args.client);
			auto newBuffer = Serializer::makeGetNamesResponse(1, "Cannot authenticate user", {});
			return Response{ std::move(newBuffer), { args.client }, msg::Type::error };
		}
		auto names = db.getUserDocumentNames(userData.username);
		auto newBuffer = Serializer::makeGetNamesResponse(1, "", names);
		return Response{ std::move(newBuffer), { args.client }, msg::Type::getDocNames };
	}

	Response Authenticator::delDoc(const ArgPack& args) {
		auto msg = Deserializer::parseDelDoc(args.buffer);
		auto userData = getUserData(args.client);
		if (userData.authToken != msg.authToken) {
			logger.logError("Cannot authenticate user", args.client);
			auto newBuffer = Serializer::makeAckResponse(msg::Type::delDoc, 1, "Cannot authenticate user");
			return Response{ std::move(newBuffer), { args.client }, msg::Type::error };
		}
		auto doc = db.getDocWithUsernameAndFilename(userData.username, msg.docFilename);
		if (!doc) {
			auto newBuffer = Serializer::makeAckResponse(msg::Type::delDoc, 1, db.getLastError());
			return Response{ std::move(newBuffer), { args.client }, msg::Type::error };
		}
		DBUser user(userData.username, "", {});
		if (!db.unlinkUserAndDoc(user, doc.value())) {
			auto newBuffer = Serializer::makeAckResponse(msg::Type::delDoc, 1, db.getLastError());
			return Response{ std::move(newBuffer), { args.client }, msg::Type::error };
		}
		auto newBuffer = Serializer::makeAckResponse(msg::Type::delDoc, 1, db.getLastError());
		return Response{ std::move(newBuffer), { args.client }, msg::Type::delDoc };
	}

	void Authenticator::clearUser(SOCKET client) {
		std::scoped_lock lock{authMutex, activeUsersMutex};
		auto it = clientToAuthToken.find(client);
		if (it == clientToAuthToken.cend()) {
			return;
		}
		activeUsers.erase(it->second.username);
		clientToAuthToken.erase(it);
	}

	void Authenticator::addUser(const SOCKET client, const std::string& authToken, const std::string& username) {
		std::scoped_lock lock{authMutex, activeUsersMutex};
		clientToAuthToken.emplace(client, UserData{ authToken, username });
		activeUsers.emplace(username, true);
	}

	bool Authenticator::checkIfUserIsActive(const std::string& username) {
		std::lock_guard lock{activeUsersMutex};
		auto it = activeUsers.find(username);
		return it != activeUsers.cend();
	}

	std::string Authenticator::getAuthToken(SOCKET client) {
		auto userData = getUserData(client);
		return userData.authToken;
	}

	Authenticator::UserData Authenticator::getUserData(SOCKET client) {
		std::lock_guard lock{authMutex};
		auto it = clientToAuthToken.find(client);
		if (it == clientToAuthToken.cend()) {
			return UserData();
		}
		return it->second;
	}
}
