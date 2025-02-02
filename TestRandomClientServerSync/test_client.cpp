#include "test_client.h"
#include "engine.h"

#include <iostream>;
#include <array>

constexpr static std::array<const char*, 20> randomStrings = { 
	"","a","b","1","2","!?","test","void","bamper","sticky","pleasure","wisconsin","starbucks","platonic",
	"creatively","paraphrase","stunningly","extravaganza","cinomatography","perplexitness"
};

TestClient::TestClient(const std::string& serverIp, const int serverPort, const int seed) :
	controller(),
	serverIp(serverIp),
	serverPort(serverPort) {
	random::Engine::get().setSeed(seed);
}

DocAction TestClient::getUndoAction() const {
	DocAction key;
	key.key.keyCode = CTRL_Z;
	key.type = msg::Type::undo;
	return key;
}

DocAction TestClient::getRandomKey() {
	std::uniform_int_distribution<> actionDist(0, 960);
	std::uniform_int_distribution<> charDist(32, 127);
	int action = random::Engine::get().getRandFromDist(actionDist);
	DocAction docAction;
	if (action < 650) {
		docAction.key.keyCode = random::Engine::get().getRandFromDist(charDist);
		docAction.type = msg::Type::write;
		docAction.text = std::string{ static_cast<char>(docAction.key.keyCode) };
	}
	else if (action < 800) {
		docAction.key.keyCode = BACKSPACE;
		docAction.type = msg::Type::erase;
		docAction.eraseSize = 1;
	}
	else if (action < 840) {
		docAction.key.keyCode = ARROW_LEFT;
		docAction.type = msg::Type::moveHorizontal;
	}
	else if (action < 880) {
		docAction.key.keyCode = ARROW_RIGHT;
		docAction.type = msg::Type::moveHorizontal;
	}
	else if (action < 920) {
		docAction.key.keyCode = ARROW_DOWN;
		docAction.type = msg::Type::moveVertical;
	}
	else if (action <= 960) {
		docAction.key.keyCode = ARROW_UP;
		docAction.type = msg::Type::moveVertical;
	}
	return docAction;
}

std::string TestClient::getRandomString() {
	std::uniform_int_distribution<> lengthDist(1, 6);
	std::uniform_int_distribution<> endlineDist(0, 1);
	std::uniform_int_distribution<> stringDist(0, 19);
	std::string endlStr = "\n";
	std::string ret;
	int length = random::Engine::get().getRandFromDist(lengthDist);
	for (int i = 0; i < length; i++) {
		bool endl = random::Engine::get().getRandFromDist(endlineDist);
		ret += randomStrings[random::Engine::get().getRandFromDist(stringDist)] + (endl ? endlStr : "");
	}
	return ret;
}

bool TestClient::makeActionSync(const DocAction& key) {
	controller.processChar(key.key);
	bool gotMsg = false;
	constexpr int maxTries = 50;
	int tries = 0;
	std::chrono::microseconds timeout{10};
	while (!gotMsg) {
		tries++;
		gotMsg = controller.checkIncomingMessages();
		if (tries > maxTries) {
			break;
		}
		std::this_thread::sleep_for(timeout);
	}
	controller.render();
	return gotMsg;
}

void TestClient::start() {
	if (!controller.connect(serverIp, serverPort)) {
		std::cout << "Connection to server failed!\n";
		return;
	}
	if (!controller.requestDocument(std::chrono::milliseconds(500), 3)) {
		std::cout << "Requesting document from the server failed!\n";
		return;
	}
}

void TestClient::stop() {
	controller.disconnect();
}