#pragma once
#include "window_base.h"

class InfoWindow : public BaseWindow {
public:
	InfoWindow(const ScrollableScreenBufferBuilder& ssbBuilder, const std::string& msg);
	InfoWindow(const ScrollableScreenBufferBuilder& ssbBuilder, const std::string&& msg);
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	std::string name() const override;
private:
	const std::string winName;
};