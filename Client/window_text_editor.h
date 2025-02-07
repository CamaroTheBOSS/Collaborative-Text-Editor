#pragma once
#include "window_base.h"

class TextEditorWindow : public BaseWindow {
	friend class SyncTester;
public:
	TextEditorWindow();
	bool processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
};
