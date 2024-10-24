#pragma once

#include "document.h"
#include "messages.h"

class Repository {
public:
	bool processMsg(msg::Buffer& buffer);
	Document& getDoc();
private:
	bool write(msg::Buffer& buffer);
	bool erase(msg::Buffer& buffer);
	bool move(msg::Buffer& buffer);

	Document doc;

};