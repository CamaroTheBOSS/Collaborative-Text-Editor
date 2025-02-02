#pragma once
#include <vector>
#include <string>
#include <Windows.h>

#include "test_client.h"

struct DocAction {
	KeyPack key{ 0, 0 };
	msg::Type type{msg::Type::error};
	int eraseSize{ 0 };
	std::string text;
	std::string str() const {
		std::stringstream ss;
		ss << "(" << type;
		if (type == msg::Type::erase) {
			ss << " " << eraseSize;
		}
		else if (type == msg::Type::write) {
			ss << " \"" + text + '"';
		}
		ss << ")";
		return ss.str();
	}
};

struct State {
	std::vector<std::vector<COORD>> cursorPositions;
	std::vector<DocAction> actions;
};