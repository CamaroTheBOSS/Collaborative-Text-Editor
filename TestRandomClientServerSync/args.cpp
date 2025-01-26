#include "args.h"

#include <iostream>;

static std::unordered_map<std::string, int> getDefaultArgs() {
	return {
		{"seed", -1}
	};
}

Args::Args() :
	args(getDefaultArgs()) {}

Args::Args(int argc, char* argv[]) :
	Args() {
	if (argc % 2 != 1) {
		std::cout << "Error when parsing arguments! Make sure all arguments have value\n";
		valid = false;
		return;
	}
	for (int i = 1; i < argc; i += 2) {
		auto msg = parseArgument(argv[i], argv[i + 1]);
		if (!msg.empty()) {
			std::cout << msg;
			valid = false;
			return;
		}
	}
}

std::string Args::parseArgument(char* argKey, char* argVal) {
	std::string key{argKey};
	if (!key.starts_with("--")) {
		return "Unrecognized argument" + std::string{argKey} + ". Did you forget --?\n";
	}
	key.erase(0, 2);
	auto it = args.find(key);
	if (it == args.end()) {
		return "Unrecognized argument" + std::string{argKey} + "\n";
	}
	try {
		int value = std::stoi(argVal);
		args[key] = value;
	}
	catch (std::exception e) {
		return "Caught exception for argument " + key + ": " + e.what() + "\n";
	}
	return "";
}