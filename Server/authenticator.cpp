#include "authenticator.h"
#include "deserializer.h"
#include "serializer.h"

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
		assert(false && "Unrecognized msg type. Aborting...");
		return Response{ buffer, {}, msg::Type::error };
	}

	Response Authenticator::loginUser(const ArgPack& args) {
		auto msg = Deserializer::parseLogin(args.buffer);
		DBUser dbUser;
		dbUser.username = std::move(msg.login);
		auto errMsg = db.getUser(dbUser);
		if (errMsg.empty()) {
			if (dbUser.password == msg.password) {
				// GENERATE TOKEN
			}
			else {
				errMsg = "Incorrect password!";
			}
		}
		auto buffer = Serializer::makeLoginResponse(msg.version, "authtoken", errMsg);
		return Response{ buffer, {args.client}, msg.type };
	}

	Response Authenticator::logoutUser(const ArgPack& args) {
		auto msg = Deserializer::parseAck(args.buffer);
		auto buffer = Serializer::makeAckResponse(msg.type, msg.version);
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
}
