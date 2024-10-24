#pragma once

#include "document.h"
#include "messages.h"

class Repository {
public:
	void processMsg(msg::Buffer& buffer);
	Document& getDoc();
private:
	void write(msg::Buffer& buffer);
	void erase(msg::Buffer& buffer);
	void move(msg::Buffer& buffer);

	Document doc;

};