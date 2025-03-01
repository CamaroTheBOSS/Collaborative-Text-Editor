#pragma once
#include "document_base.h"

class ClientSiteDocument : public BaseDocument {
public:
	ClientSiteDocument();
	ClientSiteDocument(const std::string& text);
	ClientSiteDocument(const std::string& text, const int cursors, const int myUserIdx);

	void findSegments(const std::string& pattern);
	void resetSegments();
	COORD getNextSegmentPos();
	const TextContainer::Segments& getSegments() const;
	int getChosenSegmentIndex() const;
	void setSegments(TextContainer::Segments& newSegments);
	void insertSegment(const COORD& startPos, const COORD& endPos, const int pos);
	void clearContent();
private:
	void moveSegment(std::pair<COORD, COORD>& segment, const COORD& startPos, const COORD& diff) const;
	void afterWriteAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& writtenText) override;
	void afterEraseAction(const int index, const COORD& startPos, const COORD& endPos, std::vector<std::string>& erasedText) override;

	TextContainer::Segments segments;
	int chosenSegment = -1;
};