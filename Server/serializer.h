#pragma once
#include "messages.h"
#include "server_document.h"

class Serializer {
public:
	static msg::Buffer makeConnectResponse(const ServerSiteDocument& doc, const int userIdx, const msg::Connect& msg);
	static msg::Buffer makeDisconnectResponse(const int userIdx, const msg::Disconnect& msg);
	static msg::Buffer makeWriteResponse(const COORD& startPos, const int userIdx, const msg::Write& msg);
	static msg::Buffer makeEraseResponse(const COORD& startPos, const int userIdx, const msg::Erase& msg);
	static msg::Buffer makeMoveResponse(const ServerSiteDocument& doc, const int userIdx, const msg::MoveHorizontal& msg);
	static msg::Buffer makeMoveResponse(const ServerSiteDocument& doc, const int userIdx, const msg::MoveVertical& msg);
	static msg::Buffer makeMoveResponse(const ServerSiteDocument& doc, const int userIdx, const msg::MoveSelectAll& msg);
	static msg::Buffer makeReplaceResponse(const int userIdx, const msg::Replace& msg);
private:
	static msg::Buffer makeMoveResponseImpl(const ServerSiteDocument& doc, const msg::Type type, const msg::OneByteInt version, const int userIdx, const bool withSelect);
};