#include "parser.h"

std::vector<std::string> Parser::parseTextToVector(const std::string& text) {
	size_t offset = 0;
	size_t end = text.find('\n');
	std::vector<std::string> parsedLines;
	if (end == std::string::npos) {
		parsedLines.emplace_back(text);
		return parsedLines;
	}
	do {
		std::string lineText{text.cbegin() + offset, text.cbegin() + end};
		parsedLines.emplace_back(lineText);
		offset = end + 1;
		end = text.find('\n', offset);
	} while (end != std::string::npos);
	std::string lineText{text.cbegin() + offset, text.cend()};
	parsedLines.emplace_back(lineText);
	return parsedLines;
}

std::string Parser::parseVectorToText(const std::vector<std::string>& vec) {
	std::string ret;
	for (const auto& line : vec) {
		ret += line + "\n";
	}
	if (!ret.empty()) {
		ret.erase(ret.size() - 1);
	}
	return ret;
}