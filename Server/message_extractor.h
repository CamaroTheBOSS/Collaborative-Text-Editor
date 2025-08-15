#pragma once
#include <unordered_map>

import framer;

class MessageExtractor {
public:
	std::vector<msg::Buffer> extractMessages(const SOCKET client);
	void reset(const SOCKET client);
private:
	std::unordered_map<SOCKET, Framer> clientFramerMap;
};