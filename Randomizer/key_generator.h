#pragma once


class KeyGenerator {
public:
	using Key = int;
	static Key getInvalidKey() { return -1; }
	static Key getRandomKey();
};