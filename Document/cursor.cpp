#include "cursor.h"

Cursor::Cursor() :
	_pos(COORD{ 0, 0 }),
	_offset(0) {}

Cursor::Cursor(COORD pos) :
	_pos(pos),
	_offset(pos.X) {}

void Cursor::setPosition(const COORD newPos) {
	_pos = newPos;
}

void Cursor::setOffset(int newOffset) {
	_offset = newOffset;
}

COORD Cursor::position() const {
	return _pos;
}
int Cursor::offset() const {
	return _offset;
}