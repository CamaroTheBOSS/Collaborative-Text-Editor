#pragma once
#include "messages.h"

class Deserializer {
public:
	static msg::AckMsg parseAck(const msg::Buffer& buffer);
	static msg::Login parseLogin(const msg::Buffer& buffer);
	static msg::Register parseRegister(const msg::Buffer& buffer);
	static msg::ConnectCreateDoc parseConnectCreateDoc(const msg::Buffer& buffer);
	static msg::ConnectJoinDoc parseConnectJoinDoc(const msg::Buffer& buffer);
	static msg::Disconnect parseDisconnect(const msg::Buffer& buffer);
	static msg::Write parseWrite(const msg::Buffer& buffer);
	static msg::Erase parseErase(const msg::Buffer& buffer);
	static msg::MoveHorizontal parseMoveHorizontal(const msg::Buffer& buffer);
	static msg::MoveVertical parseMoveVertical(const msg::Buffer& buffer);
	static msg::MoveTo parseMoveTo(const msg::Buffer& buffer);
	static msg::MoveSelectAll parseMoveSelectAll(const msg::Buffer& buffer);
	static msg::ControlMessage parseControlMessage(const msg::Buffer& buffer);
	static msg::Replace parseReplaceMessage(const msg::Buffer& buffer);
};