#pragma once
#include "window_base.h"

class SearchWindow : public BaseWindow {
	friend class SyncTester;
public:
	SearchWindow();
	SearchWindow(const ScrollableScreenBufferBuilder& ssbBuilder);
	ActionDone processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	std::string name() const override {
		return className;
	}
	static constexpr const char* className = "SearchWindow";
};