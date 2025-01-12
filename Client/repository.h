#pragma once

#include "document.h"
#include "messages.h"
#include "screen_buffers.h"

class Repository {
public:
	bool processMsg(msg::Buffer& buffer);
	bool saveDoc() const;
	Document& getDoc();
private:
	bool write(msg::Buffer& buffer);
	bool erase(msg::Buffer& buffer);
	bool move(msg::Buffer& buffer);
	bool sync(msg::Buffer& buffer);
	bool connectNewUser(msg::Buffer& buffer);
	bool disconnectUser(msg::Buffer& buffer);
	bool undoRedo(msg::Buffer& buffer);

	Document doc;

};