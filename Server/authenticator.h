#pragma once
#include <unordered_map>
#include <string>
#include <WinSock2.h>
#include <vector>

#include "messages.h"
#include "response.h"


namespace server {
	class Authenticator {
	public:
		Response process(SOCKET client, msg::Buffer& buffer);
	private:
		struct ArgPack {
			SOCKET client;
			msg::Buffer& buffer;
		};

		Response loginUser(const ArgPack& args);
		Response logoutUser(const ArgPack& args);
		Response registerUser(const ArgPack& args);
	};
}