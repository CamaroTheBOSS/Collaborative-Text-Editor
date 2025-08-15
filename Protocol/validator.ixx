module;

#include <string>

export module validator;

export class Validator {
public:
	static std::string validateString(const std::string& str);
};