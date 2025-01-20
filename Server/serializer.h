#pragma once
#include "messages.h"
#include "document.h"

class Serializer {
public:
	static msg::Buffer makeConnectResponse(const Document& doc, const int userIdx, const msg::Connect& msg);
	static msg::Buffer makeDisconnectResponse(const int userIdx, const msg::Disconnect& msg);
	static msg::Buffer makeWriteResponse(const COORD& startPos, const int userIdx, const msg::Write& msg);
	static msg::Buffer makeEraseResponse(const COORD& startPos, const int userIdx, const msg::Erase& msg);
	static msg::Buffer makeMoveResponse(const Document& doc, const int userIdx, const msg::MoveHorizontal& msg);
	static msg::Buffer makeMoveResponse(const Document& doc, const int userIdx, const msg::MoveVertical& msg);
	static msg::Buffer makeMoveResponse(const Document& doc, const int userIdx, const msg::MoveSelectAll& msg);
private:
	static msg::Buffer makeMoveResponseImpl(const Document& doc, const msg::Type type, const msg::OneByteInt version, const int userIdx, const bool withSelect);
};