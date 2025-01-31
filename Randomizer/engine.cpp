#include "engine.h"

namespace random {
	auto getRandomEngine(const int seed) {
		if (seed < 0) {
			std::random_device device;
			std::random_device::result_type data[(std::mt19937::state_size - 1) / sizeof(device()) + 1];
			std::generate(std::begin(data), std::end(data), std::ref(device));
			std::seed_seq seed{std::begin(data), std::end(data)};
			return std::mt19937(seed);
		}
		std::mt19937_64::result_type reproducibleSeed = seed;
		return std::mt19937(reproducibleSeed);
	}

	std::mt19937 engine = getRandomEngine(-1);

	void setSeed(const int seed) {
		engine = getRandomEngine(seed);
	}
}
