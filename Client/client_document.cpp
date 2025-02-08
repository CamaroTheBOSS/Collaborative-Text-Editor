#include "client_document.h"

ClientSiteDocument::ClientSiteDocument() :
	BaseDocument() {}

ClientSiteDocument::ClientSiteDocument(const ClientSiteDocument& other) :
	BaseDocument(other) {}

ClientSiteDocument::ClientSiteDocument(ClientSiteDocument&& other) :
	BaseDocument(std::move(other)) {}

ClientSiteDocument::ClientSiteDocument(const std::string& text) :
	BaseDocument(text) {}

ClientSiteDocument::ClientSiteDocument(const std::string& text, const int nCursors, const int myUserIdx) :
	BaseDocument(text, nCursors, myUserIdx) {}

ClientSiteDocument& ClientSiteDocument::operator=(const ClientSiteDocument& other) {
	container = other.container;
	users = other.users;
	filename = other.filename;
	myUserIdx = other.myUserIdx;
	return *this;
}

ClientSiteDocument& ClientSiteDocument::operator=(ClientSiteDocument&& other) noexcept {
	container = std::move(other.container);
	users = std::move(other.users);
	filename = std::move(other.filename);
	myUserIdx = std::move(other.myUserIdx);
	return *this;
}

TextContainer::Segments ClientSiteDocument::findSegments(const std::string& pattern) {
	return container.findAll(pattern);
}