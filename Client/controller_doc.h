#pragma once
#include "controller_base.h"

class DocController : public BaseController {
	friend class SyncTester;
public:
	DocController(TCPClient& client, Terminal& terminal);
	bool processChar(const KeyPack& key) override;
};
