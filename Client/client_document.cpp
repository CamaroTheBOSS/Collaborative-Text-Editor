#include "client_document.h"
#include "pos_helpers.h"

std::pair<int, bool> binarySearchSegments(const TextContainer::Segments& segments, const int left, const int right, const COORD& pos) {
	if (segments.empty()) {
		return { -1, false };
	}
	if (left == right) {
		return { left, false };
	}
	int center = left + (right - left) / 2;
	if (segments[center].first >= pos) {
		return binarySearchSegments(segments, left, center, pos);
	}
	else if (segments[center].second <= pos) {
		return binarySearchSegments(segments, center + 1, right, pos);
	}
	return { center, true };
}

ClientSiteDocument::ClientSiteDocument() :
	BaseDocument() {
	addUser();
}

ClientSiteDocument::ClientSiteDocument(const std::string& text) :
	BaseDocument(text) {
	addUser();
}

ClientSiteDocument::ClientSiteDocument(const std::string& text, const int nCursors, const int myUserIdx) :
	BaseDocument(text, nCursors, myUserIdx) {
	for (int i = 0; i < nCursors; i++) {
		addUser();
	}
}

void ClientSiteDocument::findSegments(const std::string& pattern) {
	segments = container.findAll(pattern);
	chosenSegment = -1;
}

void ClientSiteDocument::resetSegments() {
	segments.clear();
	chosenSegment = -1;
}

void ClientSiteDocument::setCursorOnNextSegmentStart(const int index) {
	if (!validateUserIdx(index) || segments.empty()) {
		return;
	}
	chosenSegment++;
	if (chosenSegment >= segments.size()) {
		chosenSegment = 0;
	}
	setCursorPos(index, segments[chosenSegment].first);
}

const TextContainer::Segments& ClientSiteDocument::getSegments() const {
	return segments;
}

int ClientSiteDocument::getChosenSegmentIndex() const {
	return chosenSegment;
}

void ClientSiteDocument::moveSegment(std::pair<COORD, COORD>& segment, const COORD& startPos, const COORD& diff) const {
	if (segment.first.Y == startPos.Y && segment.first.X >= startPos.X) {
		segment.first = segment.first + diff;
		segment.second = segment.second + diff;
	}
	else if (segment.first.Y > startPos.Y) {
		segment.first = segment.first + COORD{ 0, diff.Y };
		segment.second = segment.second + COORD{ 0, diff.Y };
	}
}

void ClientSiteDocument::afterWriteAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& writtenText) {
	if (segments.empty()) {
		return;
	}
	COORD diff = endPos - startPos;
	auto [closestSegment, toDelete] = binarySearchSegments(segments, 0, segments.size(), startPos);
	int start = (std::max)(closestSegment, 0);
	for (int i = start; i < segments.size(); i++) {
		moveSegment(segments[i], startPos, diff);
	}
	if (toDelete) {
		segments.erase(segments.cbegin() + closestSegment);
		if (chosenSegment == closestSegment) {
			chosenSegment--;
		}
	}
}

void ClientSiteDocument::afterEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& erasedText) {
	if (segments.empty()) {
		return;
	}
	COORD diff = endPos - startPos;
	auto [closestSegmnetToEnd, toDeleteEnd] = binarySearchSegments(segments, 0, segments.size(), endPos);
	int start = (std::max)(closestSegmnetToEnd, 0);
	std::vector<int> indexesToDelete;
	for (int i = start; i < segments.size(); i++) {
		if (segmentsIntersect(segments[i].first, segments[i].second, endPos, startPos)) {
			indexesToDelete.push_back(i);
			continue;
		}
		moveSegment(segments[i], startPos, diff);
	}
	if (!indexesToDelete.empty()) {
		segments.erase(segments.cbegin() + indexesToDelete.front(), segments.cbegin() + indexesToDelete.back());
		if (chosenSegment >= indexesToDelete.front() && chosenSegment <= indexesToDelete.back()) {
			chosenSegment = indexesToDelete.front() - 1;
		}
	}
}
