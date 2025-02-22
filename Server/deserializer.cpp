#include "deserializer.h"

msg::ConnectCreateDoc Deserializer::parseConnectCreateDoc(const msg::Buffer& buffer) {
	msg::ConnectCreateDoc msg;
	msg::parse(buffer, 0, msg.type, msg.version, msg.socket, msg.filename);
	return msg;
}
msg::ConnectLoadDoc Deserializer::parseConnectLoadDoc(const msg::Buffer& buffer) {
	msg::ConnectLoadDoc msg;
	msg::parse(buffer, 0, msg.type, msg.version, msg.socket, msg.acCode);
	return msg;
}
msg::ForwardConnect Deserializer::parseMasterForwardConnect(const msg::Buffer& buffer) {
	msg::ForwardConnect msg;
	msg::parse(buffer, 0, msg.type, msg.version, msg.socket, msg.docCode);
	return msg;
}
msg::Connect Deserializer::parseConnect(const msg::Buffer& buffer) {
	msg::Connect msg;
	msg::parse(buffer, 0, msg.type, msg.version);
	return msg;
}
msg::Disconnect Deserializer::parseDisconnect(const msg::Buffer& buffer) {
	msg::Disconnect msg;
	msg::parse(buffer, 0, msg.type, msg.version);
	return msg;
}
msg::Write Deserializer::parseWrite(const msg::Buffer& buffer) {
	auto msg = msg::Write{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.text);
	return msg;
}
msg::Erase Deserializer::parseErase(const msg::Buffer& buffer) {
	auto msg = msg::Erase{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.eraseSize);
	return msg;
}
msg::MoveHorizontal Deserializer::parseMoveHorizontal(const msg::Buffer& buffer) {
	auto msg = msg::MoveHorizontal{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.side, msg.withSelect);
	return msg;
}
msg::MoveVertical Deserializer::parseMoveVertical(const msg::Buffer& buffer) {
	auto msg = msg::MoveVertical{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.side, msg.clientWidth, msg.withSelect);
	return msg;
}
msg::MoveTo Deserializer::parseMoveTo(const msg::Buffer& buffer) {
	auto msg = msg::MoveTo{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.X, msg.Y);
	return msg;
}
msg::MoveSelectAll Deserializer::parseMoveSelectAll(const msg::Buffer& buffer) {
	auto msg = msg::MoveSelectAll{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token);
	return msg;
}
msg::ControlMessage Deserializer::parseControlMessage(const msg::Buffer& buffer) {
	auto msg = msg::ControlMessage{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token);
	return msg;
}

msg::Replace Deserializer::parseReplaceMessage(const msg::Buffer& buffer) {
	auto msg = msg::Replace{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token, msg.text, msg.segments);
	return msg;
}