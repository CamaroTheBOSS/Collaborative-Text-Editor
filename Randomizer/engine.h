#pragma once
#include <random>

namespace random {
	extern std::mt19937 engine;
	void setSeed(const int seed);
}
