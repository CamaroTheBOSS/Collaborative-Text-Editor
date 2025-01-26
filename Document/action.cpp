#include "action.h"
#include "pos_helpers.h"
#include "parser.h"

Action::Action(ActionType type, COORD& startPos, std::vector<std::string>& text) :
	type(type),
	start(Cursor(startPos)),
	text(std::move(text)),
	timestamp(std::chrono::system_clock::now()) {}

Action::Action(ActionType type, COORD& startPos, std::vector<std::string>& text, Timestamp timestamp):
	type(type),
	start(Cursor(startPos)),
	text(std::move(text)),
	timestamp(timestamp) {}

COORD Action::getTextSize() const {
	auto textSize = COORD{ 0, 0 };
	if (!text.empty()) {
		textSize.Y = text.size();
		if (!text[text.size() - 1].empty()) {
			textSize.X = text[text.size() - 1].size();
		}
	}
	return textSize;
}

ActionType Action::getType() const {
	return type;
}

COORD Action::getStartPos() const {
	return start.position();
}

std::string Action::getText() const {
	return Parser::parseVectorToText(text);
}

Timestamp Action::getTimestamp() const {
	return timestamp;
}

std::vector<std::string> Action::splitText(const COORD& splitPoint) {
	if (splitPoint > getTextSize() - COORD{1, 1} || splitPoint < COORD{0, 0} || splitPoint.X < 0) {
		return {};
	}
	std::vector<std::string> secondPart;
	int startX = (std::min)(splitPoint.X, static_cast<SHORT>(text[splitPoint.Y].size()));
	secondPart.emplace_back(text[splitPoint.Y].substr(startX));
	for (int i = splitPoint.Y + 1; i < text.size(); i++) {
		secondPart.emplace_back(std::move(text[i]));
	}
	text[splitPoint.Y].erase(startX);
	text.erase(text.begin() + splitPoint.Y + 1, text.end());
	return secondPart;
}

std::vector<std::string>& Action::mergeText(std::vector<std::string>& firstText, std::vector<std::string>& otherText) {
	if (otherText.empty()) {
		return firstText;
	}
	if (firstText.empty()) {
		firstText = std::move(otherText);
		return firstText;
	}
	firstText[firstText.size() - 1] += otherText[0];
	for (int i = 1; i < otherText.size(); i++) {
		firstText.emplace_back(std::move(otherText[i]));
	}
	return firstText;
}
