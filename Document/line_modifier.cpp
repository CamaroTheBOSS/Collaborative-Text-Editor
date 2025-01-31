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

std::pair<int, std::string> LineModifier::erase(std::string& line, const int pos, const int n) {
	if (pos <= 0 || pos > line.size() || n <= 0) {
		return { pos, "" };
	}
	const int nLetters = (std::min)(pos, n);
	auto erasedText = LineModifier::cut(line, pos - nLetters, pos);
	return { pos - nLetters, std::move(erasedText) };
}

std::string LineModifier::cut(std::string& line, const int left, const int right) {
	auto [effLeft, effRight] = effectiveRange(line, left, right);
	if (effLeft >= effRight || line.empty()) {
		return "";
	}
	std::string str{line.cbegin() + effLeft, line.cbegin() + effRight};
	line.erase(line.cbegin() + effLeft, line.cbegin() + effRight);
	return str;
}

std::string_view LineModifier::get(const std::string& line, const int left, const int right) {
	auto [effLeft, effRight] = effectiveRange(line, left, right);
	if (effLeft >= effRight || line.empty()) {
		return "";
	}
	return std::string_view{line.cbegin() + effLeft, line.cbegin() + effRight};
}

std::pair<int, int> LineModifier::effectiveRange(const std::string& line, const int left, const int right) {
	return { (std::max)(0, left), (std::min)(static_cast<int>(line.size()), right) };
}

bool LineModifier::endlPresent(const std::string& line) {
	return !line.empty() && line[line.size() - 1] == '\n';
}