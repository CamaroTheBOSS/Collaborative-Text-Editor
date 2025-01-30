#pragma once
#include "Windows.h"

class Cursor {
public:
	Cursor();
	Cursor(COORD pos);
	void setPosition(const COORD newPos);
	void setOffset(int newOffset);
	COORD position() const;
	int offset() const;
	bool operator==(const Cursor& other) const;

private:
	COORD _pos;
	int _offset;
};