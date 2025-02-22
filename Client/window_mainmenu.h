#pragma once
#include "window_base.h"

class MainMenuWindow : public BaseWindow {
	friend class SyncTester;
public:
	MainMenuWindow(const ScrollableScreenBufferBuilder& ssbBuilder);
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	std::string name() const override {
		return className;
	}
	static constexpr const char* className = "Main Menu";
private:
	Event goUp();
	Event goDown();
	void selectOption();
	Event executeOption();
	int selectedOption = 0;
	int numOptions = 3;
};