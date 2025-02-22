#pragma once
#include "window_base.h"

class LoadDocWindow : public BaseWindow {
public:
	LoadDocWindow(const ScrollableScreenBufferBuilder& ssbBuilder);
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	std::string name() const override {
		return className;
	}
	static constexpr const char* className = "LoadDocWindow";
};