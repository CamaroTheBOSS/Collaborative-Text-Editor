#pragma once
#include <string>

#define ARROW_UP 1296
#define ARROW_DOWN 1304
#define ARROW_LEFT 1299
#define ARROW_RIGHT 1301

#define CTRL_ARROW_UP 1365
#define CTRL_ARROW_DOWN 1369

#define CTRL_A 1
#define CTRL_C 3
#define CTRL_V 22
#define CTRL_X 24
#define CTRL_Z 26
#define CTRL_R 18
#define CTRL_F 6

#define BACKSPACE 8
#define TABULAR 9
#define ENTER 13
#define ESC 27

#define F3 1061

struct KeyPack {
	int keyCode;
	bool shiftPressed;
	std::string str() const {
		return "(" + std::to_string(keyCode) + (shiftPressed ? ", shift)" : ")");
	}
};