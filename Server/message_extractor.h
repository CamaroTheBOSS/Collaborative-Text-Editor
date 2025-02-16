#pragma once
#include <unordered_map>

#include "framer.h"

class MessageExtractor {
public:
	std::vector<msg::Buffer> extractMessages(const SOCKET client);
	void reset(const SOCKET client);
private:
	std::unordered_map<SOCKET, Framer> clientFramerMap;
};