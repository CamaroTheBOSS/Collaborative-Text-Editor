#pragma once
#include <functional>
#include "window_base.h"

class ReplaceWindow : public BaseWindow {
	friend class SyncTester;
public:
	ReplaceWindow(const ScrollableScreenBufferBuilder& ssbBuilder);
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	std::string name() const override {
		return className;
	}
	static constexpr const char* className = "Replace Window";
};