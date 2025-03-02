#include "authenticator.h"
#include "deserializer.h"
#include "serializer.h"
#include "engine.h"

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
		}
		return Response{ buffer, {}, msg::Type::error };
	}

	Response Authenticator::loginUser(const ArgPack& args) {
		auto msg = Deserializer::parseLogin(args.buffer);
		DBUser dbUser;
		dbUser.username = std::move(msg.login);
		auto errMsg = db.getUser(dbUser);
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
		auto errMsg = db.putUser(dbUser);
		auto buffer = Serializer::makeRegisterResponse(msg.version, errMsg);
		return Response{ buffer, {args.client}, msg.type };
	}

	void Authenticator::clearUser(SOCKET client) {
		std::scoped_lock lock{authMutex, activeUsersMutex};
		auto it = clientToAuthToken.find(client);
		assert(it != clientToAuthToken.cend());
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
