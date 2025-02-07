#pragma once
#include "window_base.h"

class TextEditorWindow : public BaseWindow {
	friend class SyncTester;
public:
	TextEditorWindow(TCPClient& client);
	bool processChar(const KeyPack& key, const std::string& clipboardData) override;
};
