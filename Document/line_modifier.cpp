#include "line_modifier.h"

SHORT LineModifier::insert(std::string& line, const int pos, const std::string_view newText) {
	if (pos < 0 || pos > line.size() || newText.empty()) {
		return pos;
	}
	line.insert(pos, std::string{ newText });
	return pos + newText.size();
}

int LineModifier::append(std::string& line, const std::string_view newText) {
	line.append(std::string{ newText });
	return line.size();
}

int LineModifier::erase(std::string& line, const int pos, const int n) {
	if (pos <= 0 || pos > line.size() || n <= 0) {
		return pos;
	}
	const int nLetters = (std::min)(pos, n);
	line.erase(line.cbegin() + pos - nLetters, line.cbegin() + pos);
	return pos - nLetters;
}

std::string LineModifier::cut(std::string& line, const int left, const int right) {
	if (left >= right || line.empty()) {
		return "";
	}
	auto [effLeft, effRight] = effectiveRange(line, left, right);
	std::string str{line.cbegin() + effLeft, line.cbegin() + effRight};
	line.erase(line.cbegin() + effLeft, line.cbegin() + effRight);
	return str;
}

std::string_view LineModifier::get(std::string& line, const int left, const int right) {
	if (left >= right || line.empty()) {
		return "";
	}
	auto [effLeft, effRight] = effectiveRange(line, left, right);
	return std::string_view{line.cbegin() + effLeft, line.cbegin() + effRight};
}

std::pair<int, int> LineModifier::effectiveRange(std::string& line, const int left, const int right) {
	return { (std::max)(0, left), (std::min)(static_cast<int>(line.size()), right) };
}