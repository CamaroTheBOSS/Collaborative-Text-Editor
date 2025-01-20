#include "deserializer.h"

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
msg::MoveSelectAll Deserializer::parseMoveSelectAll(const msg::Buffer& buffer) {
	auto msg = msg::MoveSelectAll{};
	msg::parse(buffer, 0, msg.type, msg.version, msg.token);
	return msg;
}