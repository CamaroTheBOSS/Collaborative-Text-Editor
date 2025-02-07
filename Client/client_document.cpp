#include "client_document.h"

ClientSiteDocument::ClientSiteDocument() :
	BaseDocument() {}

ClientSiteDocument::ClientSiteDocument(const std::string& text) :
	BaseDocument(text) {}

ClientSiteDocument::ClientSiteDocument(const std::string& text, const int nCursors, const int myUserIdx) :
	BaseDocument(text, nCursors, myUserIdx) {}