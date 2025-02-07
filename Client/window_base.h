#pragma once
#include "tcp_client.h"
#include "screen_buffers.h"
#include "keypack.h"

class BaseWindow {
	friend class SyncTester;
public:
	BaseWindow();
	virtual bool processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData = "") = 0;
	bool saveDoc() const;
	ClientSiteDocument& getDoc();
	ScrollableScreenBuffer& getBuffer();
	unsigned int getDocBufferWidth();
protected:
	ClientSiteDocument doc;
	ScrollableScreenBuffer buffer;
};
