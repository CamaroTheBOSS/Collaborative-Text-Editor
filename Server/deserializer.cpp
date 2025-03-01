#include "deserializer.h"

msg::AckMsg Deserializer::parseAck(const msg::Buffer& buffer) {
	msg::AckMsg msg;
	msg::parse(buffer, 0, msg.type, msg.version);
	return msg;
}
msg::Login Deserializer::parseLogin(const msg::Buffer& buffer) {
	msg::Login msg;
	msg::parse(buffer, 0, msg.type, msg.version, msg.login, msg.password);
	return msg;
}
msg::Register Deserializer::parseRegister(const msg::Buffer& buffer) {
	msg::Register msg;
	msg::parse(buffer, 0, msg.type, msg.version, msg.login, msg.password);
	return msg;
}
msg::ConnectCreateDoc Deserializer::parseConnectCreateDoc(const msg::Buffer& buffer) {
	msg::ConnectCreateDoc msg;
	msg::parse(buffer, 0, msg.type, msg.version, msg.socket, msg.filename);
	return msg;
}
msg::ConnectJoinDoc Deserializer::parseConnectJoinDoc(const msg::Buffer& buffer) {
	msg::ConnectJoinDoc msg;
	msg::parse(buffer, 0, msg.type, msg.version, msg.socket, msg.acCode);
	return msg;
}
msg::Disconnect Deserializer::parseDisconnect(const msg::Buffer& buffer) {
	msg::Disconnect msg;
	msg::parse(buffer, 0, msg.type, msg.version, msg.authToken);
	return msg;
}
msg::Write Deserializer::parseWrite(const msg::Buffer& buffer) {
	auto msg = msg::Write{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.authToken, msg.text);
	return msg;
}
msg::Erase Deserializer::parseErase(const msg::Buffer& buffer) {
	auto msg = msg::Erase{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.authToken, msg.eraseSize);
	return msg;
}
msg::MoveHorizontal Deserializer::parseMoveHorizontal(const msg::Buffer& buffer) {
	auto msg = msg::MoveHorizontal{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.authToken, msg.side, msg.withSelect);
	return msg;
}
msg::MoveVertical Deserializer::parseMoveVertical(const msg::Buffer& buffer) {
	auto msg = msg::MoveVertical{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.authToken, msg.side, msg.clientWidth, msg.withSelect);
	return msg;
}
msg::MoveTo Deserializer::parseMoveTo(const msg::Buffer& buffer) {
	auto msg = msg::MoveTo{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.authToken, msg.X, msg.Y);
	return msg;
}
msg::MoveSelectAll Deserializer::parseMoveSelectAll(const msg::Buffer& buffer) {
	auto msg = msg::MoveSelectAll{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.authToken);
	return msg;
}
msg::ControlMessage Deserializer::parseControlMessage(const msg::Buffer& buffer) {
	auto msg = msg::ControlMessage{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.authToken);
	return msg;
}

msg::Replace Deserializer::parseReplaceMessage(const msg::Buffer& buffer) {
	auto msg = msg::Replace{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.authToken, msg.text, msg.segments);
	return msg;
}