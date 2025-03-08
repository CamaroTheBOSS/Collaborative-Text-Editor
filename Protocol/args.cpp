#include "args.h"
#include <iostream>
#include <vector>
#include <format>

Args::Args(ArgsMap&& config, std::vector<Command>&& commands) :
	args(std::move(config)),
	commands(std::move(commands)) {}

std::string Args::getDescription() {
	std::string description{"Commands:\n"};
	for (const auto& command : commands) {
		description += std::format("{}: {}\n   Required args: ", command.key, command.description);
		for (const auto& key : command.requiredArgKeys) {
			description += key + ", ";
		}
		description += "\n";
	}
	description += "Arguments:\n";
	for (const auto& [key, arg] : args) {
		switch (arg.type) {
		case Type::integer:
			description += std::format("--{}: {}\n", key, arg.description);
			break;
		case Type::string:
			description += std::format("--{}: {}\n", key, arg.description);
			break;
		}
	}
	return description;
}

std::string Args::parse(int argc, char* argv[]) {
	if (argc == 2) {
		auto msg = parseCommand(argv[1]);
		if (!msg.empty()) {
			valid = false;
			return msg + "\n";
		}
	}
	std::string errMsg;
	for (int i = 2; i < argc; i += 2) {
		auto msg = parseArgument(argv[i - 1], argv[i]);
		if (msg == "COMMAND") {
			msg = parseCommand(argv[i - 1]);
			i -= 1;
		}
		if (!msg.empty()) {
			valid = false;
			return msg + "\n";
		}
	}
	for (const auto& [key, arg] : args) {
		auto isArgNull = arg.value.index() == to_underlying(Type::null);
		auto requiredForCommand = setCommandIdx >= 0 &&
			commands[setCommandIdx].isArgRequiredForCommand(key);
		if (isArgNull && requiredForCommand) {
			valid = false;
			return "Required argument '--" + key + "' not found!\n";
		}
		else if (isArgNull && setCommandIdx < 0) {
			return "Required argument '--" + key + "' not found!\n";
		}
		
	}
	return "";
}

std::string Args::parseCommand(char* argVal) {
	auto val = std::string{ argVal };
	if (val.starts_with("--")) {
		return "No value provied to argument '" + val + "'\n";
	}
	command = std::move(val);
	for (int i = 0; i < commands.size(); i++) {
		if (commands[i].key == command) {
			setCommandIdx = i;
			return "";
		}
	}
	return "Unrecognized command '" + command + "'";
}

std::string Args::parseArgument(char* argKey, char* argVal) {
	std::string key{argKey};
	if (!key.starts_with("--")) {
		if (command != commandRun) {
			return "Unrecognized argument '" + std::string{argKey} + "'. Did you forget --?\n";
		}
		return "COMMAND";
	}
	key.erase(0, 2);
	auto it = args.find(key);
	if (it == args.end()) {
		return "Unrecognized argument '" + std::string{argKey} + "'\n";
	}
	try {
		auto& existingObj = args[key];
		switch (existingObj.type) {
		case Type::null:
			throw std::exception("Null argument type error");
		case Type::integer:
			args[key].value = std::stoi(argVal);
			break;
		case Type::string:
			args[key].value = argVal;
			break;
		default:
			throw std::exception("Unrecognized argument type error");
		}	
	}
	catch (std::exception e) { 
		return "Caught exception for argument '--" + key + "': " + e.what() + "\n";
	}
	return "";
}