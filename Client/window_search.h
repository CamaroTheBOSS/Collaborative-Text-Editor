#pragma once
#include <functional>
#include "window_base.h"

class SearchWindow : public BaseWindow {
	friend class SyncTester;
public:
	SearchWindow(const ScrollableScreenBufferBuilder& ssbBuilder);
	Event onDelete() override;
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	std::string name() const override {
		return className;
	}
	static constexpr const char* className = "Search Window";
};