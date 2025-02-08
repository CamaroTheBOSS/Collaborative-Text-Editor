#pragma once
#include "window_base.h"

class TextEditorWindow : public BaseWindow {
	friend class SyncTester;
public:
	TextEditorWindow();
	TextEditorWindow(const Pos<double>& leftTop, const Pos<double>& rightBottom, const Pos<int>& consoleSize);
	ActionDone processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
};
