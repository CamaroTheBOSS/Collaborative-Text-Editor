#pragma once
#include "tcp_client.h"
#include "screen_buffers_builder.h"
#include "keypack.h"

enum class ActionDone { done, undone, up, down, render };

class BaseWindow {
	friend class SyncTester;
public:
	BaseWindow();
	BaseWindow(const ScrollableScreenBufferBuilder& ssbBuilder);
	virtual ActionDone processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData = "") = 0;
	bool saveDoc() const;
	ClientSiteDocument& getDoc();
	ScrollableScreenBuffer& getBuffer();
	unsigned int getDocBufferWidth();
	bool isActive() const;
	void activate();
	void deactivate();
	virtual std::string name() const = 0;
protected:
	ClientSiteDocument doc;
	ScrollableScreenBuffer buffer;
	bool active;
};
