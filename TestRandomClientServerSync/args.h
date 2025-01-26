#pragma once
#include <string>
#include <assert.h>
#include <unordered_map>

class Args {
public:
	Args();
	Args(int argc, char* argv[]);
	bool isValid() { return valid; }
	int operator[](const std::string& key) { return args[key]; };
private:
	std::string parseArgument(char* argKey, char* argVal);
	std::unordered_map<std::string, int> args;
	bool valid = true;
};