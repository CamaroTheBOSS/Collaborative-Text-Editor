#pragma once
#include <optional>
#include <unordered_map>
#include "engine.h"

template<typename T>
class Storage {
public:
	using Key = random::Engine::Key;
	Key push(T& object) {
		Key key = random::Engine::get().getRandomKey();
		storage.emplace(key, std::move(object));
		return key;
	}

	bool erase(const Key key) {
		return storage.erase(key);
	}

	void erase(const std::vector<Key>& keys) {
		for (const auto& key : keys) {
			storage.erase(key);
		}
	}

	void erase(typename std::unordered_map<Key, T>::const_iterator it) {
		storage.erase(it);
	}

	auto find(const Key key) const {
		return storage.find(key);
	}

	auto cbegin() const {
		return storage.cbegin();
	}

	auto cend() const {
		return storage.cend();
	}

private:
	std::unordered_map<Key, T> storage;
};