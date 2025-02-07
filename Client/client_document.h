#pragma once
#include "document_base.h"

class ClientSiteDocument : public BaseDocument {
public:
	ClientSiteDocument();
	ClientSiteDocument(const ClientSiteDocument& other);
	ClientSiteDocument(ClientSiteDocument&& other);
	ClientSiteDocument(const std::string& text);
	ClientSiteDocument(const std::string& text, const int cursors, const int myUserIdx);
	ClientSiteDocument& operator=(const ClientSiteDocument& other);
	ClientSiteDocument& operator=(ClientSiteDocument&& other) noexcept;
private:
};