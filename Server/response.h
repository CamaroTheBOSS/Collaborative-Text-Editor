#pragma once
#include "messages.h"

#include <vector>

namespace server {
	struct Response {
		msg::Buffer buffer;
		std::vector<SOCKET> destinations;
		msg::Type msgType;
	};
}