#pragma once
#include "window_base.h"

class InfoWindow : public BaseWindow {
public:
	InfoWindow(const ScrollableScreenBufferBuilder& ssbBuilder, const std::string& title, const std::string& msg);
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	std::string name() const override {
		return className;
	}
	const std::string className;
};