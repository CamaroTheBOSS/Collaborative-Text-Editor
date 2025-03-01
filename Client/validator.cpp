#include "validator.h"

#include <algorithm>

std::string Validator::validateString(const std::string& str) {
	if (str.empty()) {
		return "Field cannot be empty!";
	}
	if (str.size() > 64) {
		return "Field cannot be longer than 64 letters!";
	}
	if (str.size() < 3) {
		return "Field cannot be shorter than 3 letters!";
	}
	std::string forbidden = R"(./\<>;:+='")";
	bool found = std::any_of(forbidden.cbegin(), forbidden.cend(), [&](auto letter) { 
			return (str.find(letter) != std::string::npos); 
		});
	if (found) {
		return "Field contains fobidden letters! Any of " + forbidden + " cannot be used.";
	}
	return "";
}