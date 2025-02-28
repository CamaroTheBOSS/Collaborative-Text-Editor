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
		std::string authToken;
		if (errMsg.empty()) {
			if (dbUser.password == msg.password) {
				authToken = random::Engine::get().getRandomString(16);
				std::lock_guard lock{authMutex};
				clientToAuthToken.emplace(args.client, authToken);
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
		std::lock_guard lock{authMutex};
		clientToAuthToken.erase(client);
	}

	std::string Authenticator::getAuthToken(SOCKET client) {
		std::lock_guard lock{authMutex};
		auto it = clientToAuthToken.find(client);
		if (it == clientToAuthToken.cend()) {
			return "";
		}
		return it->second;
	}
}
