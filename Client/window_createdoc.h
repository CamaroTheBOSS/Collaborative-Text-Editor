#pragma once
#include "window_base.h"

class CreateDocWindow : public BaseWindow {
public:
	CreateDocWindow(const ScrollableScreenBufferBuilder& ssbBuilder);
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	std::string name() const override {
		return className;
	}
	static constexpr const char* className = "CreateDocWindow";
};