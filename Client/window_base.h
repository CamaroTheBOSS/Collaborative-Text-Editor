#pragma once
#include "tcp_client.h"
#include "screen_buffers.h"
#include "keypack.h"

enum class ActionDone { done, undone, up, down };

class BaseWindow {
	friend class SyncTester;
public:
	BaseWindow();
	BaseWindow(const Pos<double>& leftTop, const Pos<double>& rightBottom, const Pos<int>& consoleSize);
	virtual ActionDone processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData = "") = 0;
	bool saveDoc() const;
	ClientSiteDocument& getDoc();
	ScrollableScreenBuffer& getBuffer();
	unsigned int getDocBufferWidth();
protected:
	ClientSiteDocument doc;
	ScrollableScreenBuffer buffer;
};
