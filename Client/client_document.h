#pragma once
#include "document_base.h"

class ClientSiteDocument : public BaseDocument {
public:
	ClientSiteDocument();
	ClientSiteDocument(const std::string& text);
	ClientSiteDocument(const std::string& text, const int cursors, const int myUserIdx);
private:

};