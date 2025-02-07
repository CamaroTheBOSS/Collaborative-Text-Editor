#pragma once
#include "tcp_client.h"
#include "repository.h"
#include "terminal.h"

class BaseController {
	friend class SyncTester;
public:
	BaseController(TCPClient& client, Terminal& terminal);
	virtual bool processChar(const KeyPack& key) = 0;
	bool saveDoc() const;
	ClientSiteDocument& getDoc();
protected:
	TCPClient& client;
	Terminal& terminal;
	ClientSiteDocument doc;
};
