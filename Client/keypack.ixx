module;

#include <string>

export module keypack;

export struct KeyPack {
	int keyCode;
	bool shiftPressed;
	std::string str() const {
		return "(" + std::to_string(keyCode) + (shiftPressed ? ", shift)" : ")");
	}
};