#pragma once
#include "args.h"
#include "application.h"

static constexpr const char* port = "port";
static constexpr const char* ip = "ip";
static constexpr const char* login = "login";
static constexpr const char* password = "password";
static constexpr const char* acCode = "access-code";
static constexpr const char* filename = "filename";

static constexpr const char* help = "help";
static constexpr const char* run = "run";
static constexpr const char* registration = "register";
static constexpr const char* join = "join";
static constexpr const char* create = "create";

void doAndRender(Application& app, int action) {
	app.processChar(KeyPack{ action, false });
	app.render();
}

void goDownInMenuAndAccept(Application& app, int nPositions) {
	for (int i = 0; i < nPositions; i++) {
		doAndRender(app, ARROW_DOWN);
	}
	doAndRender(app, ENTER);
}

void passStringToWindow(Application& app, const std::string& str) {
	for (auto it = str.cbegin(); it != str.cend(); it++) {
		doAndRender(app, *it);
	}
}

void passCredentials(Application& app, Args& args) {
	auto& loginArg = args.get<std::string>(login);
	passStringToWindow(app, loginArg);
	doAndRender(app, CTRL_ARROW_DOWN);
	auto& passArg = args.get<std::string>(password);
	passStringToWindow(app, passArg);
	doAndRender(app, ENTER);
}

void scenarioRegisterUser(Application& app, Args& args) {
	goDownInMenuAndAccept(app, 1);
	passCredentials(app, args);
}

void scenarioLoginUser(Application& app, Args& args) {
	doAndRender(app, ENTER);
	passCredentials(app, args);
}

void scenarioJoinDoc(Application& app, Args& args) {
	scenarioLoginUser(app, args);

	// Close information window, focus on Join option and click it
	doAndRender(app, ESC);
	goDownInMenuAndAccept(app, 3);

	// Pass accode
	auto& accessCode = args.get<std::string>(acCode);
	passStringToWindow(app, accessCode);
	doAndRender(app, ENTER);
	doAndRender(app, ESC);
}

void scenarioCreateDoc(Application& app, Args& args) {
	scenarioLoginUser(app, args);

	// Close information window, focus on Join option and click it
	doAndRender(app, ESC);
	goDownInMenuAndAccept(app, 1);

	// Create doc
	auto& docName = args.get<std::string>(filename);
	passStringToWindow(app, docName);
	doAndRender(app, ENTER);
	doAndRender(app, ESC);
}