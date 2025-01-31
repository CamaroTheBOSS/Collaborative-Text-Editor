#include "engine.h"
#include "key_generator.h"

KeyGenerator::Key KeyGenerator::getRandomKey() {
	std::uniform_int_distribution<> dist(0, INT_MAX);
	Key key = dist(random::engine);
	return key;
}