#include "parser.h"

std::vector<std::string> Parser::parseLineToVector(const std::string& line, const char delimiter) {
	int prev = 0, curr = 0;
	std::vector<std::string> parsedLines;
	while (curr = line.find(delimiter, prev), curr != std::string::npos) {
		parsedLines.emplace_back(line.substr(prev, curr - prev));
		prev = curr + 1;
	}
	auto subline = line.substr(prev);
	if (subline.size() > 0) {
		parsedLines.emplace_back(std::move(subline));
	}
	return parsedLines;
}

std::vector<std::string> Parser::parseTextToVector(const std::string& text, const char delimiter) {
	size_t offset = 0;
	size_t end = text.find(delimiter);
	std::vector<std::string> parsedLines;
	if (end == std::string::npos) {
		parsedLines.emplace_back(text);
		return parsedLines;
	}
	do {
		bool windowsStyleEndl = end > 0 && text[end - 1] == '\r';
		std::string lineText{text.cbegin() + offset, text.cbegin() + end - windowsStyleEndl};
		parsedLines.emplace_back(lineText);
		offset = end + 1;
		end = text.find(delimiter, offset);
	} while (end != std::string::npos);
	std::string lineText{text.cbegin() + offset, text.cend()};
	parsedLines.emplace_back(lineText);
	postprocess(parsedLines);
	return parsedLines;
}

std::string Parser::parseVectorToText(const std::vector<std::string>& vec, const char delimiter) {
	std::string ret;
	for (const auto& line : vec) {
		ret += line + delimiter;
	}
	if (!ret.empty()) {
		ret.erase(ret.size() - 1);
	}
	return ret;
}

void Parser::postprocess(std::vector<std::string>& text) {
	for (auto& line : text) {
		int pos = 0;
		while (pos < text.size()) {
			pos = line.find('\t', pos);
			if (pos == std::string::npos) {
				break;
			}
			line.replace(pos, 1, "    ");
		}
	}
}