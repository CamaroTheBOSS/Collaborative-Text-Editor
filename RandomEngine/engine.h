#ifndef RANDOM_DLL
#define RANDOM_DLL
#pragma once
#pragma warning( disable : 4251 )
#include <random>
#include <mutex>

#ifdef RANDOMENGINE_EXPORTS
#define RANDOM_API __declspec(dllexport)
#else
#define RANDOM_API __declspec(dllimport)
#endif

namespace random {

	class RANDOM_API Engine {
	public:
		using Key = int;
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;
		static Engine& get();

		//Random number gen
		void setSeed(const int seed);
		int getRandFromDist(std::uniform_int_distribution<>& dist);

		//Keygen
		Key getInvalidKey() const;
		Key isKeyInvalid(const Key key) const;
		Key getRandomKey();
	private:
		Engine() = default;
		std::mt19937 engine;
		std::mutex engineLock;
	};
}
#endif