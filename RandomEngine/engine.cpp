#include "pch.h"
#include "engine.h"

#include <random>

namespace random {

	static auto getRandomEngine(const int seed) {
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

	Engine& Engine::get() {
		static Engine instance;
		return instance;
	}

	void Engine::setSeed(const int seed) {
		std::scoped_lock lock{engineLock};
		engine = getRandomEngine(seed);
	}

	//Random number gen
	int Engine::getRandFromDist(std::uniform_int_distribution<>& dist) {
		std::scoped_lock lock{engineLock};
		return dist(engine);
	}

	//Keygen
	Engine::Key Engine::getInvalidKey() const { 
		return -1; 
	}
	Engine::Key Engine::isKeyInvalid(const Key key) const { 
		return key == getInvalidKey(); 
	};
	Engine::Key Engine::getRandomKey() {
		std::scoped_lock lock{engineLock};
		std::uniform_int_distribution<> dist(0, INT_MAX);
		Key key = dist(engine);
		return key;
	}
}